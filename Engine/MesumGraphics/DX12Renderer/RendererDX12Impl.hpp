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
    void          execute() override;
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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class mSynchToolDX12 : public render::mISynchTool
{
   public:
    void init(Desc& a_desc) final;
    void destroy() final;

   public:
    mUInt             currentFenceIndex;
    std::vector<mU64> fenceValues = {};
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class mRenderTargetDX12 : public render::mIRenderTarget
{
   public:
    D3D12_CPU_DESCRIPTOR_HANDLE rtv;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class mSwapchainDX12 final : public render::mISwapchain
{
   public:
    ~mSwapchainDX12() final = default;

    void init_win32(Desc const& a_desc, DescWin32 const& a_descWin32) final;
    void init_x11(Desc const& a_config, Descx11 const& a_data) final;
    void destroy() final;

    void resize(mU32 a_width, mU32 a_height) final;

   public:
    [[nodiscard]] IDXGISwapChain4* get_swapchain() const;
    [[nodiscard]] mUInt            get_currentBackBufferIndex() const;
    [[nodiscard]] ID3D12Resource* get_backbuffer(mUInt a_backbufferIndex) const;
    [[nodiscard]] CD3DX12_CPU_DESCRIPTOR_HANDLE get_rtv(
        mInt a_backbufferIndex) const;

   private:
    void update_renderTargetViews();

   private:
    DXGI_SWAP_CHAIN_DESC1 m_descSwapChain;

    ComPtr<IDXGISwapChain4>      m_pSwapChain;
    std::vector<ID3D12Resource*> m_backbuffers;
    // TODO remove ComPtr
    ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap;
    mUInt                        m_descriptorSize;

    // By default, enable V-Sync.
    mBool m_vSync            = true;
    mBool m_tearingSupported = false;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class mApiDX12 final : public render::mIApi
{
   public:
    static memory::mMemoryType      sm_memoryType;
    static memory::mObjectAllocator sm_mal;

   public:
    ~mApiDX12() final = default;

    void init() final;
    void destroy() final;

    void start_dearImGuiNewFrameRenderer() const final;

    [[nodiscard]] render::mISwapchain& create_swapchain() const final;
    void destroy_swapchain(render::mISwapchain& a_swapchain) const final;

    [[nodiscard]] render::Taskset& create_renderTaskset() const final;
    void destroy_renderTaskset(render::Taskset& a_taskset) const final;

    [[nodiscard]] render::mISynchTool& create_synchTool() const final;
    void destroy_synchTool(render::mISynchTool& a_synchTool) const final;
};

}  // namespace m::dx12

#endif  // M_RendererDX12Impl