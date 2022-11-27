#pragma once

#include <MesumGraphics/DX12Renderer/DX12RendererCommon.hpp>
#include <MesumGraphics/Renderer.hpp>

namespace m::dx12
{
namespace DX12Surface
{
const mUInt scm_numFrames = 3;  // TODO remove this
};

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
class mSynchTool : public render::mISynchTool
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
class mRenderTarget : public render::mIRenderTarget
{
   public:
    D3D12_CPU_DESCRIPTOR_HANDLE rtv;
    mU32                        width;
    mU32                        height;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class mSwapchain final : public render::mISwapchain
{
   public:
    ~mSwapchain() final = default;

    void init_win32(Desc const& a_desc, DescWin32 const& a_descWin32) final;
    void init_x11(Desc const& a_config, Descx11 const& a_data) final;
    void destroy() final;

    void resize(mU32 a_width, mU32 a_height) final;

    Desc const& get_desc() override { return m_currentDesc; }

   public:
    [[nodiscard]] IDXGISwapChain4* get_swapchain() const;
    [[nodiscard]] mUInt            get_currentBackBufferIndex() const;
    [[nodiscard]] ID3D12Resource* get_backbuffer(mUInt a_backbufferIndex) const;
    [[nodiscard]] CD3DX12_CPU_DESCRIPTOR_HANDLE get_rtv(
        mInt a_backbufferIndex) const;

   private:
    void update_renderTargetViews();

   private:
    Desc m_currentDesc;

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
class mApi final : public render::mIApi
{
   public:
    static memory::mMemoryType      sm_memoryType;
    static memory::mObjectAllocator sm_mal;

   public:
    ~mApi() final = default;

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