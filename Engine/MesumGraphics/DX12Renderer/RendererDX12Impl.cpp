#include <RendererDX12Impl.hpp>

#include <DX12Context.hpp>
#include <imgui_impl_dx12.h>

namespace m
{
namespace dx12
{
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
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

void DX12Surface::init_x11(render::X11SurfaceInitData& a_data)
{
    // X11 not supported with DX12
    mHardAssert(false);
}

void DX12Surface::init_dearImGui(Callback<void>& a_callback)
{
    m_isHoldingDearImgui                                      = true;
    DX12Context::gs_dx12Contexte->m_dearImGuiPlatImplCallback = a_callback;

    m_SRVDescriptorHeap = create_descriptorHeap(
        DX12Context::gs_dx12Contexte->m_device,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, scm_numFrames,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

    ImGui_ImplDX12_Init(
        DX12Context::gs_dx12Contexte->m_device.Get(), scm_numFrames,
        DXGI_FORMAT_B8G8R8A8_UNORM, m_SRVDescriptorHeap.Get(),
        m_SRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        m_SRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
}

void DX12Surface::destroy()
{
    if (m_isHoldingDearImgui)
    {
        ImGui_ImplDX12_Shutdown();
    }
    DX12Context::gs_dx12Contexte->get_commandQueue().flush();
}

void DX12Surface::render()
{
    DX12Context::gs_dx12Contexte->get_commandQueue().wait_fenceValue(
        m_frameFenceValues[m_currentBackBufferIndex]);

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
        if (!m_isHoldingDearImgui)
        {
            clearColor[0] = 0.9f;
            clearColor[2] = 0.4f;
        }

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
            m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
            m_currentBackBufferIndex, m_RTVDescriptorSize);

        graphicCommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);

        if (m_isHoldingDearImgui)
        {
            graphicCommandList->OMSetRenderTargets(1, &rtv, FALSE, NULL);
            graphicCommandList->SetDescriptorHeaps(
                1, m_SRVDescriptorHeap.GetAddressOf());
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(),
                                          graphicCommandList.Get());
        }
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
}

void DX12Surface::resize(U32 a_width, U32 a_height)
{
    if (m_clientWidth != a_width || m_clientHeight != a_height)
    {
        // Don't allow 0 size swap chain back buffers.
        m_clientWidth  = std::max(1u, a_width);
        m_clientHeight = std::max(1u, a_height);

        DX12Context::gs_dx12Contexte->get_commandQueue().flush();

        if (m_isHoldingDearImgui)
        {
            ImGui_ImplDX12_InvalidateDeviceObjects();
        }

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
        if (m_isHoldingDearImgui)
        {
            ImGui_ImplDX12_CreateDeviceObjects();
        }
    }
}

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


//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
void DX12Renderer::init()
{
    DX12Context::gs_dx12Contexte = new DX12Context();
    DX12Context::gs_dx12Contexte->init();
}

void DX12Renderer::destroy()
{
    DX12Context::gs_dx12Contexte->deinit();
    delete DX12Context::gs_dx12Contexte;
#ifdef M_DEBUG
    dx12::report_liveObjects();
#endif  // M_DEBUG
}

void DX12Renderer::start_dearImGuiNewFrame()
{
    ImGui_ImplDX12_NewFrame();
    DX12Context::gs_dx12Contexte->m_dearImGuiPlatImplCallback.call();
}

render::ISurface* DX12Renderer::get_newSurface()
{
    return new DX12Surface();
}

}
}  // namespace m