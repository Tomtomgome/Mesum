#include <imgui_impl_dx12.h>

#include <DX12Context.hpp>
#include <RendererDX12Impl.hpp>

namespace m::dx12
{
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
render::Task* DX12RenderTaskset::add_task(render::TaskData* a_data)
{
    auto task = a_data->getNew_dx12Implementation(a_data);
    m_set_tasks.push_back(task);
    return task;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DX12RenderTaskset::clear()
{
    for (auto task : m_set_tasks) { delete task; }
    m_set_tasks.clear();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DX12Surface::init_win32(render::Win32SurfaceInitData& a_data)
{
    m_clientWidth  = a_data.m_width;
    m_clientHeight = a_data.m_height;

    m_tearingSupported = DX12Context::gs_dx12Contexte->get_tearingSupport();

    m_swapChain =
        create_swapChain(a_data.m_hwnd,
                         DX12Context::gs_dx12Contexte->get_commandQueue()
                             .get_D3D12CommandQueue(),
                         m_clientWidth, m_clientHeight, scm_numFrames);
    mDXGIDebugNamed(m_swapChain, "Window SwapChain");

    m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    m_RTVDescriptorHeap =
        create_descriptorHeap(DX12Context::gs_dx12Contexte->m_device,
                              D3D12_DESCRIPTOR_HEAP_TYPE_RTV, scm_numFrames);
    mD3D12DebugNamed(m_RTVDescriptorHeap, "Window descriptor heap");

    m_RTVDescriptorSize =
        DX12Context::gs_dx12Contexte->m_device
            ->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    update_renderTargetViews(DX12Context::gs_dx12Contexte->m_device,
                             m_swapChain, m_RTVDescriptorHeap);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DX12Surface::init_x11(render::X11SurfaceInitData& a_data)
{
    // X11 not supported with DX12
    mAssert(false);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DX12Surface::destroy()
{
    for (auto taskset : m_renderTasksets)
    {
        taskset->clear();
        delete taskset;
    }

    DX12Context::gs_dx12Contexte->get_commandQueue().flush();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE DX12Surface::get_currentRtvDesc()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
        m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        m_currentBackBufferIndex, m_RTVDescriptorSize);
    return rtv;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
render::Taskset* DX12Surface::addNew_renderTaskset()
{
    auto taskset = new DX12RenderTaskset();
    m_renderTasksets.push_back(taskset);
    return taskset;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DX12Surface::render()
{
    auto backBuffer = m_backBuffers[m_currentBackBufferIndex];

    ComPtr<ID3D12GraphicsCommandList2> graphicCommandList =
        DX12Context::gs_dx12Contexte->get_commandQueue().get_commandList();
    // Clear the render target.
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET);

        graphicCommandList->ResourceBarrier(1, &barrier);

        Float clearColor[] = {0.4f, 0.6f, 0.9f, 1.0f};

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
            m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
            m_currentBackBufferIndex, m_RTVDescriptorSize);

        graphicCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);

        DX12Context::gs_dx12Contexte->get_commandQueue().execute_commandList(
            graphicCommandList);

        for (auto taskset : m_renderTasksets)
        {
            for (const auto task : taskset->m_set_tasks) { task->prepare(); }
            for (const auto task : taskset->m_set_tasks) { task->execute(); }
        }

        graphicCommandList =
            DX12Context::gs_dx12Contexte->get_commandQueue().get_commandList();
    }
    // Present
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT);
        graphicCommandList->ResourceBarrier(1, &barrier);

        DX12Context::gs_dx12Contexte->get_commandQueue().execute_commandList(
            graphicCommandList);

        UINT syncInterval = get_syncInterval();  // m_vSync ? 1 : 0;
        UINT presentFlags =
            get_presentFlags();  // m_tearingSupported && !m_vSync ?
                                 // DXGI_PRESENT_ALLOW_TEARING : 0;
        check_MicrosoftHRESULT(
            m_swapChain->Present(syncInterval, presentFlags));

        m_frameFenceValues[m_currentBackBufferIndex] =
            DX12Context::gs_dx12Contexte->get_commandQueue().signal_fence();

        m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
    }

    DX12Context::gs_dx12Contexte->get_commandQueue().wait_fenceValue(
        m_frameFenceValues[m_currentBackBufferIndex]);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DX12Surface::resize(U32 a_width, U32 a_height)
{
    if (m_clientWidth != a_width || m_clientHeight != a_height)
    {
        // Don't allow 0 size swap chain back buffers.
        m_clientWidth  = std::max(1u, a_width);
        m_clientHeight = std::max(1u, a_height);

        DX12Context::gs_dx12Contexte->get_commandQueue().flush();

        for (Int i = 0; i < scm_numFrames; ++i)
        {
            // Any references to the back buffers must be released
            // before the swap chain can be resized.
            m_backBuffers[i].Reset();
            m_frameFenceValues[i] =
                m_frameFenceValues[m_currentBackBufferIndex];
        }

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        check_MicrosoftHRESULT(m_swapChain->GetDesc(&swapChainDesc));
        check_MicrosoftHRESULT(m_swapChain->ResizeBuffers(
            scm_numFrames, a_width, a_height, swapChainDesc.BufferDesc.Format,
            swapChainDesc.Flags));

        m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
        update_renderTargetViews(DX12Context::gs_dx12Contexte->m_device,
                                 m_swapChain, m_RTVDescriptorHeap);
    }
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
void DX12Surface::update_renderTargetViews(
    ComPtr<ID3D12Device2> a_device, ComPtr<IDXGISwapChain4> a_swapChain,
    ComPtr<ID3D12DescriptorHeap> a_descriptorHeap)
{
    UInt size_rtvDescriptor = a_device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        a_descriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (int i = 0; i < scm_numFrames; ++i)
    {
        ComPtr<ID3D12Resource> backBuffer;
        check_MicrosoftHRESULT(
            a_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        a_device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

        m_backBuffers[i] = backBuffer;

        rtvHandle.Offset(size_rtvDescriptor);
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DX12Renderer::init()
{
    DX12Context::gs_dx12Contexte = new DX12Context();
    DX12Context::gs_dx12Contexte->init();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DX12Renderer::destroy()
{
    DX12Context::gs_dx12Contexte->deinit();
    delete DX12Context::gs_dx12Contexte;
#ifdef M_DEBUG
    dx12::report_liveObjects();
#endif  // M_DEBUG
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DX12Renderer::start_dearImGuiNewFrameRenderer() const
{
    ImGui_ImplDX12_NewFrame();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
render::ISurface* DX12Renderer::getNew_surface()
{
    return new DX12Surface();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
render::IResource* DX12Renderer::getNew_texture()
{
    return nullptr;//new DX12Texture();
}

}  // namespace m::dx12