#include "RendererDX12Impl.hpp"

namespace m::dx12
{
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSwapchainDX12::init_win32(Desc const&      a_desc,
                                DescWin32 const& a_descWin32)
{
    m_tearingSupported = DX12Context::gs_dx12Contexte->get_tearingSupport();

    // Swapchain creation
    ComPtr<IDXGISwapChain4> dxgiSwapChain4;
    ComPtr<IDXGIFactory4>   dxgiFactory4;
    UINT                    flags_createFactory = 0;
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

    // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
    // will be handled manually.
    check_mhr(dxgiFactory4->MakeWindowAssociation(a_descWin32.hwd,
                                                  DXGI_MWA_NO_ALT_ENTER));

    check_mhr(swapChain1.As(&dxgiSwapChain4));
    mDXGIDebugNamed(dxgiSwapChain4, "Suplied SwapChain");

    m_pSwapChain = dxgiSwapChain4.Get();
    // TODO : Get the window name
    mDXGIDebugNamed(m_pSwapChain, "Window SwapChain");

    //    m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
    //
    //    m_RTVDescriptorHeap =
    //        create_descriptorHeap(DX12Context::gs_dx12Contexte->m_device,
    //                              D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
    //                              scm_numFrames);
    //    mD3D12DebugNamed(m_RTVDescriptorHeap, "Window descriptor heap");
    //
    //    m_RTVDescriptorSize =
    //        DX12Context::gs_dx12Contexte->m_device
    //            ->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    //
    //    update_renderTargetViews(DX12Context::gs_dx12Contexte->m_device,
    //                             m_swapChain, m_RTVDescriptorHeap);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSwapchainDX12::init_x11(Desc const& a_config, Descx11 const& a_data)
{
    // X11 not supported with DX12
    mAssert(false);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSwapchainDX12::resize(mU32 a_width, mU32 a_heigh) {}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSwapchainDX12::destroy()
{
    m_pSwapChain->Release();
}
}  // namespace m::dx12