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
struct DX12RenderTaskset : public render::Taskset
{
    std::vector<render::Task*> m_set_tasks;

    render::Task* add_task(render::TaskData* a_data) override;
    void          clear() override;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class DX12Surface : public render::ISurface
{
   public:
    ~DX12Surface() override = default;

    void init_win32(render::Win32SurfaceInitData& a_data) override;
    void init_x11(render::X11SurfaceInitData& a_data) override;

    render::Taskset* addNew_renderTaskset() override;

    void render() override;
    void resize(mU32 a_width, mU32 a_height) override;

    void destroy() override;

    D3D12_CPU_DESCRIPTOR_HANDLE get_currentRtvDesc();

    mU32 get_width() const { return m_clientWidth; }
    mU32 get_height() const { return m_clientHeight; }

   public:
    // The number of swap chain back buffers.
    static const mU8 scm_numFrames = 3;

   private:
    [[nodiscard]] mUInt get_syncInterval() const { return m_vSync ? 1 : 0; }

    [[nodiscard]] mUInt get_presentFlags() const
    {
        return m_tearingSupported && !m_vSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    }

    void update_renderTargetViews(
        ComPtr<ID3D12Device2> a_device, ComPtr<IDXGISwapChain4> a_swapChain,
        ComPtr<ID3D12DescriptorHeap> a_descriptorHeap);

   private:
    // By default, enable V-Sync.
    // Can be toggled with the V key.
    mBool m_vSync            = true;
    mBool m_tearingSupported = false;

    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12Resource>  m_backBuffers[scm_numFrames];
    mUInt                   m_currentBackBufferIndex;

    ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
    mUInt                        m_RTVDescriptorSize;

    ComPtr<ID3D12DescriptorHeap> m_SRVDescriptorHeap;

    std::vector<DX12RenderTaskset*> m_renderTasksets;

    // Synchronization objects
    mU64 m_frameFenceValues[scm_numFrames] = {};

    // Surface description
    mU32 m_clientWidth;
    mU32 m_clientHeight;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class DX12Renderer : public render::IRenderer
{
   public:
    void init() override;
    void destroy() override;

    mBool get_supportDearImGuiMultiViewports() override { return true; }
    void  start_dearImGuiNewFrameRenderer() const override;

    render::ISurface*  getNew_surface() override;
    render::IResource* getNew_texture() override;
};

}  // namespace m::dx12

#endif  // M_RendererDX12Impl