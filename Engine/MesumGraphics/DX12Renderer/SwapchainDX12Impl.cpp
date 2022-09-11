#include "RendererDX12Impl.hpp"

#include "DX12Context.hpp"

namespace m::dx12
{
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSwapchain::init_win32(Desc const& a_desc, DescWin32 const& a_descWin32)
{
    m_tearingSupported = DX12Context::gs_dx12Contexte->get_tearingSupport();

    // Swapchain creation
    ComPtr<IDXGIFactory4> dxgiFactory4;
    UINT                  flags_createFactory = 0;
#ifdef M_DEBUG
    flags_createFactory = DXGI_CREATE_FACTORY_DEBUG;
#endif  // M_DEBUG

    check_mhr(
        CreateDXGIFactory2(flags_createFactory, IID_PPV_ARGS(&dxgiFactory4)));
    mDXGIDebugNamed(dxgiFactory4, "SwapChain Factory");

    // Create a descriptor for the swap chain.
    m_descSwapChain             = {};
    m_descSwapChain.Width       = a_desc.width;
    m_descSwapChain.Height      = a_desc.height;
    m_descSwapChain.Format      = DXGI_FORMAT_B8G8R8A8_UNORM;
    m_descSwapChain.Stereo      = false;
    m_descSwapChain.SampleDesc  = {1, 0};
    m_descSwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    m_descSwapChain.BufferCount = a_desc.bufferCount;
    m_descSwapChain.Scaling     = DXGI_SCALING_STRETCH;
    m_descSwapChain.SwapEffect  = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    m_descSwapChain.AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED;
    m_descSwapChain.Flags =
        check_tearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

    // Create a swap chain for the window.
    ComPtr<IDXGISwapChain1> swapChain1;
    check_mhr(dxgiFactory4->CreateSwapChainForHwnd(
        DX12Context::gs_dx12Contexte->get_commandQueue()
            .get_D3D12CommandQueue()
            .Get(),
        a_descWin32.hwd, &m_descSwapChain, nullptr, nullptr, &swapChain1));
    mDXGIDebugNamed(swapChain1, "Base SwapChain");

    // Disable the Alt+Enter fullscreen toggle feature.
    check_mhr(dxgiFactory4->MakeWindowAssociation(a_descWin32.hwd,
                                                  DXGI_MWA_NO_ALT_ENTER));

    check_mhr(swapChain1.As(&m_pSwapChain));
    mDXGIDebugNamed(m_pSwapChain, "Suplied SwapChain");

    // TODO : Get the window name
    mDXGIDebugNamed(m_pSwapChain, "Window SwapChain");

    m_pDescriptorHeap = create_descriptorHeap(
                            DX12Context::gs_dx12Contexte->m_device,
                            D3D12_DESCRIPTOR_HEAP_TYPE_RTV, a_desc.bufferCount)
                            .Get();
    mD3D12DebugNamed(m_pDescriptorHeap, "Swapchain descriptor heap");

    m_descriptorSize =
        DX12Context::gs_dx12Contexte->m_device
            ->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    update_renderTargetViews();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSwapchain::init_x11(Desc const& a_config, Descx11 const& a_data)
{
    // X11 not supported with DX12
    mAssert(false);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSwapchain::destroy()
{
    for (auto& buffer : m_backbuffers) { buffer->Release(); }

    // m_pSwapChain->Release();

    std::vector<ID3D12Resource*>().swap(m_backbuffers);
    m_pDescriptorHeap->Release();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSwapchain::resize(mU32 a_width, mU32 a_height)
{
    // We assume synchronization is done externaly and the swapchain can be
    // safely resized
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    check_mhr(m_pSwapChain->GetDesc1(&swapChainDesc));
    if (swapChainDesc.Width != a_width || swapChainDesc.Height != a_height)
    {
        DX12Context::gs_dx12Contexte->get_commandQueue().flush();

        for (mInt i = 0; i < swapChainDesc.BufferCount; ++i)
        {
            // Any references to the back buffers must be released
            // before the swap chain can be resized.
            m_backbuffers[i]->Release();
            m_backbuffers[i] = nullptr;
        }

        check_mhr(m_pSwapChain->ResizeBuffers(
            swapChainDesc.BufferCount, std::max(1u, a_width),
            std::max(1u, a_height), swapChainDesc.Format, swapChainDesc.Flags));

        update_renderTargetViews();
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
IDXGISwapChain4* mSwapchain::get_swapchain() const
{
    return m_pSwapChain.Get();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mUInt mSwapchain::get_currentBackBufferIndex() const
{
    return m_pSwapChain->GetCurrentBackBufferIndex();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ID3D12Resource* mSwapchain::get_backbuffer(mUInt a_backbufferIndex) const
{
    return m_backbuffers[a_backbufferIndex];
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CD3DX12_CPU_DESCRIPTOR_HANDLE mSwapchain::get_rtv(mInt a_backbufferIndex) const
{
    return {m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
            a_backbufferIndex, m_descriptorSize};
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void mSwapchain::update_renderTargetViews()
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    check_mhr(m_pSwapChain->GetDesc(&swapChainDesc));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    m_backbuffers.resize(swapChainDesc.BufferCount);
    for (int i = 0; i < m_backbuffers.size(); ++i)
    {
        check_mhr(m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_backbuffers[i])));

        DX12Context::gs_dx12Contexte->m_device->CreateRenderTargetView(
            m_backbuffers[i], nullptr, rtvHandle);

        rtvHandle.Offset(mInt(m_descriptorSize));
    }
}

}  // namespace m::dx12