#include <DX12Window.hpp>

namespace m
{
namespace dx12
{
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
void DX12Window::init(HWND a_hwnd, U32 a_width, U32 a_height)
{
    m_clientWidth  = a_width;
    m_clientHeight = a_height;

    m_tearingSupported = DX12Context::gs_dx12Contexte.get_tearingSupport();

    m_swapChain =
        create_swapChain(a_hwnd, DX12Context::gs_dx12Contexte.m_commandQueue,
                         a_width, a_height, scm_numFrames);

    m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    m_RTVDescriptorHeap =
        create_descriptorHeap(DX12Context::gs_dx12Contexte.m_device,
                              D3D12_DESCRIPTOR_HEAP_TYPE_RTV, scm_numFrames);
    m_RTVDescriptorSize =
        DX12Context::gs_dx12Contexte.m_device->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    update_renderTargetViews(DX12Context::gs_dx12Contexte.m_device, m_swapChain,
                             m_RTVDescriptorHeap);

    for (Int i = 0; i < scm_numFrames; ++i)
    {
        m_commandAllocators[i] =
            create_commandAllocator(DX12Context::gs_dx12Contexte.m_device,
                                    D3D12_COMMAND_LIST_TYPE_DIRECT);
    }
    m_commandList =
        create_commandList(DX12Context::gs_dx12Contexte.m_device,
                           m_commandAllocators[m_currentBackBufferIndex],
                           D3D12_COMMAND_LIST_TYPE_DIRECT);

    m_fence      = create_fence(DX12Context::gs_dx12Contexte.m_device);
    m_fenceEvent = create_eventHandle();
}

void DX12Window::destroy()
{
    flush(DX12Context::gs_dx12Contexte.m_commandQueue, m_fence, m_fenceValue,
          m_fenceEvent);

    CloseHandle(m_fenceEvent);
}

void DX12Window::render()
{
    auto commandAllocator = m_commandAllocators[m_currentBackBufferIndex];
    auto backBuffer       = m_backBuffers[m_currentBackBufferIndex];

    commandAllocator->Reset();
    m_commandList->Reset(commandAllocator.Get(), nullptr);
    // Clear the render target.
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET);

        m_commandList->ResourceBarrier(1, &barrier);
        Float                         clearColor[] = {0.4f, 0.6f, 0.9f, 1.0f};
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
            m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
            m_currentBackBufferIndex, m_RTVDescriptorSize);

        m_commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    }
    // Present
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT);
        m_commandList->ResourceBarrier(1, &barrier);

        check_MicrosoftHRESULT(m_commandList->Close());

        ID3D12CommandList* const commandLists[] = {m_commandList.Get()};
        DX12Context::gs_dx12Contexte.m_commandQueue->ExecuteCommandLists(
            _countof(commandLists), commandLists);

        UINT syncInterval = get_syncInterval();  // m_vSync ? 1 : 0;
        UINT presentFlags =
            get_presentFlags();  // m_tearingSupported && !m_vSync ?
                                 // DXGI_PRESENT_ALLOW_TEARING : 0;
        check_MicrosoftHRESULT(
            m_swapChain->Present(syncInterval, presentFlags));

        m_frameFenceValues[m_currentBackBufferIndex] = signal_fence(
            DX12Context::gs_dx12Contexte.m_commandQueue, m_fence, m_fenceValue);
        m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

        wait_fenceValue(m_fence, m_frameFenceValues[m_currentBackBufferIndex],
                        m_fenceEvent);
    }
}

void DX12Window::resize(U32 a_width, U32 a_height)
{
    if (m_clientWidth != a_width || m_clientHeight != a_height)
    {
        // Don't allow 0 size swap chain back buffers.
        m_clientWidth  = std::max(1u, a_width);
        m_clientHeight = std::max(1u, a_height);

        flush(DX12Context::gs_dx12Contexte.m_commandQueue, m_fence,
              m_fenceValue, m_fenceEvent);

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
        update_renderTargetViews(DX12Context::gs_dx12Contexte.m_device,
                                 m_swapChain, m_RTVDescriptorHeap);
    }
}

void DX12Window::update_renderTargetViews(
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
}  // namespace dx12
}  // namespace m