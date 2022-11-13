#pragma once

#include "MesumGraphics/Renderer.hpp"
#include "VulkanRendererCommon.hpp"

namespace m::vulkan
{
namespace VulkanSurface
{
const mUInt scm_numFrames = 3;  // TODO remove this
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct VulkanRenderTaskset : public render::Taskset
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
    VkImageView imageView;
    // Tmp hack for vulkan dynamic rendering
    mU32 width;
    mU32 height;
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

    // ----
    void          acquire_image(mUInt a_waitIndex);
    void          present(mUInt a_presentIndex);
    mRenderTarget get_currentRenderTarget();
    VkImage       get_currentImage()
    {
        return m_swapChainImages[m_currentImageIndex];
    }

    VkSemaphore get_acquiredImageSemaphore(mU64 a_index)
    {
        return m_semaphoresImageAcquired[a_index];
    }
    VkSemaphore get_renderCompletedSemaphore(mU64 a_index)
    {
        return m_semaphoresRenderCompleted[a_index];
    }

   private:
    void _init(Desc const& a_desc);
    void _init_swapChain();

    void _destroy_swapChain();

   private:
    Desc m_currentDesc;

    VkSurfaceKHR   m_surface   = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;

    mU32                     m_currentImageIndex = 0;
    std::vector<VkSemaphore> m_semaphoresImageAcquired;
    std::vector<VkSemaphore> m_semaphoresRenderCompleted;

    std::vector<VkImage>     m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;

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

}  // namespace m::vulkan
