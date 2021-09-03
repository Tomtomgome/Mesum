#ifndef M_RendererVulkanImpl
#define M_RendererVulkanImpl
#pragma once

#include <MesumGraphics/Renderer.hpp>
#include <VulkanRendererCommon.hpp>

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
    void resize(U32 a_width, U32 a_height) override;

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
    U32          get_width() const { return m_clientWidth; }
    U32          get_height() const { return m_clientHeight; }

   public:
    // The number of swap chain back buffers.
    static const U8 scm_numFrames = 3;
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

    std::vector<U64>         m_tstpRenderFinish;
    std::vector<VkSemaphore> m_semaphoresImageAcquired;
    std::vector<VkSemaphore> m_semaphoresRenderCompleted;

    // By default, enable V-Sync.
    // Can be toggled with the V key.
    Bool m_vSync            = true;
    Bool m_tearingSupported = false;

    UInt m_currentBackBufferIndex = 0;
    UInt m_currentImageIndex      = 0;

    // Synchronization objects
    U64 m_frameFenceValues[scm_numFrames] = {};

    // Surface description
    U32 m_clientWidth;
    U32 m_clientHeight;

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

    Bool get_supportDearImGuiMultiViewports() override { return false; }
    void start_dearImGuiNewFrameRenderer() const override;

    render::ISurface* get_newSurface() override;
};

}  // namespace m::vulkan

#endif  // M_RendererVulkanImpl