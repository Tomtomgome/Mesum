#ifndef M_RendererDX12Impl
#define M_RendererDX12Impl
#pragma once

#include <MesumGraphics/DX12Renderer/DX12RendererCommon.hpp>
#include <MesumGraphics/Renderer.hpp>

namespace m::dx12
{
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class DX12Surface : public render::ISurface
{
   public:
    ~DX12Surface() override = default;

    void init_win32(render::Win32SurfaceInitData& a_data) override;
    void init_x11(render::X11SurfaceInitData& a_data) override;

    void init_dearImGui(const Callback<void>& a_callback) override;

    void render() override;
    void resize(U32 a_width, U32 a_height) override;

    void destroy() override;

    // The number of swap chain back buffers.
    static const U8 scm_numFrames = 3;

   private:
    [[nodiscard]] UInt get_syncInterval() const { return m_vSync ? 1 : 0; }

    [[nodiscard]] UInt get_presentFlags() const
    {
        return m_tearingSupported && !m_vSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    }

    void update_renderTargetViews(
        ComPtr<ID3D12Device2> a_device, ComPtr<IDXGISwapChain4> a_swapChain,
        ComPtr<ID3D12DescriptorHeap> a_descriptorHeap);

   private:

    // By default, enable V-Sync.
    // Can be toggled with the V key.
    Bool m_vSync              = true;
    Bool m_tearingSupported   = false;
    Bool m_isHoldingDearImgui = false;

    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12Resource>  m_backBuffers[scm_numFrames];
    UInt                    m_currentBackBufferIndex;

    ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
    UInt                         m_RTVDescriptorSize;

    ComPtr<ID3D12DescriptorHeap> m_SRVDescriptorHeap;

    // Synchronization objects
    U64 m_frameFenceValues[scm_numFrames] = {};

    // Surface description
    U32 m_clientWidth;
    U32 m_clientHeight;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class DX12Renderer : public render::IRenderer
{
   public:
    void init() override;
    void destroy() override;

    Bool get_supportDearImGuiMultiViewports() override { return true; }
    void start_dearImGuiNewFrame() override;

    render::ISurface* get_newSurface() override;
};

}  // namespace m

#endif  // M_RendererDX12Impl