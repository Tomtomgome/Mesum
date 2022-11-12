#pragma once

#include "MesumGraphics/Renderer.hpp"
#include "VulkanRendererCommon.hpp"

namespace m::vulkan
{
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
class VulkanSurface : public render::ISurface
{
   public:
    ~VulkanSurface() override = default;

    void init_win32(render::Win32SurfaceInitData& a_data) override;
    void init_x11(render::X11SurfaceInitData& a_data) override;

    render::Taskset* addNew_renderTaskset() override;

    void render() override;
    void resize(mU32 a_width, mU32 a_height) override;

    void destroy() override;

    VkCommandBuffer get_currentCommandBuffer()
    {
        return m_frameMainCommandBuffers[m_currentBackBufferIndex];
    }
    VkFramebuffer get_currentFramebuffer()
    {
        return m_frameFramebuffers[m_currentImageIndex];
    }
    VkRenderPass get_mainRenderPass() { return m_mainRenderPass; }
    mU32         get_width() const { return m_clientWidth; }
    mU32         get_height() const { return m_clientHeight; }

   public:
    // The number of swap chain back buffers.
    static const mU8      scm_numFrames = 3;
    static const VkFormat scm_selectedSwapChainFormat =
        VK_FORMAT_B8G8R8A8_UNORM;

   private:
    void init_internal();
    void init_swapChain();

    void destroy_swapChain();

   private:
    VkSurfaceKHR             m_surface   = VK_NULL_HANDLE;
    VkSwapchainKHR           m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage>     m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;

    std::vector<mU64>        m_tstpRenderFinish;
    std::vector<VkSemaphore> m_semaphoresImageAcquired;
    std::vector<VkSemaphore> m_semaphoresRenderCompleted;

    // By default, enable V-Sync.
    // Can be toggled with the V key.
    mBool m_vSync            = true;
    mBool m_tearingSupported = false;

    mUInt m_currentBackBufferIndex = 0;
    mUInt m_currentImageIndex      = 0;

    // Synchronization objects
    mU64 m_frameFenceValues[scm_numFrames] = {};

    // Surface description
    mU32 m_clientWidth;
    mU32 m_clientHeight;

    // Base render objects
    VkCommandPool              m_frameCommandPools[scm_numFrames];
    VkCommandBuffer            m_frameMainCommandBuffers[scm_numFrames];
    VkRenderPass               m_mainRenderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> m_frameFramebuffers;

    std::vector<VulkanRenderTaskset*> m_renderTasksets;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class VulkanRenderer : public render::IRenderer
{
   public:
    void init() override;
    void destroy() override;

    mBool get_supportDearImGuiMultiViewports() override { return false; }
    void  start_dearImGuiNewFrameRenderer() const override;

    render::ISurface*  getNew_surface() override;
    render::IResource* getNew_texture() override;
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

    std::vector<VkSemaphore> semaphoresImageAcquired;
    std::vector<VkSemaphore> semaphoresRenderCompleted;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class mRenderTarget : public render::mIRenderTarget
{
   public:
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
    void acquire_image(VkSemaphore& a_semaphoreToWaitOn);
    void present(VkSemaphore& a_semaphoreToWaitOn);

    // ---
    [[NODISCARD]] mU32 get_currentImageIndex() const
    {
        return m_currentImageIndex;
    }

   private:
    void _init(Desc const& a_desc);
    void _init_swapChain();

    void _destroy_swapChain();

   private:
    Desc m_currentDesc;

    VkSurfaceKHR   m_surface   = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;

    mU32 m_currentImageIndex = 0;

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
