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
class VulkanSurface : public render::ISurface
{
   public:
    ~VulkanSurface() override = default;

    void init_win32(render::Win32SurfaceInitData& a_data) override;
    void init_x11(render::X11SurfaceInitData& a_data) override;

    void init_dearImGui(Callback<void> const& a_callback) override;

    void render() override;
    void resize(U32 a_width, U32 a_height) override;

    void destroy() override;

   private:
    void init_internal();
    void init_swapChain();

    void destroy_swapChain();

   private:
    // The number of swap chain back buffers.
    static const U8       scm_numFrames = 3;
    static const VkFormat scm_selectedSwapChainFormat =
        VK_FORMAT_B8G8R8A8_UNORM;

    VkSurfaceKHR             m_surface   = VK_NULL_HANDLE;
    VkSwapchainKHR           m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage>     m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;

    std::vector<U64>         m_tstpRenderFinish;
    std::vector<VkSemaphore> m_semaphoresImageAcquired;
    std::vector<VkSemaphore> m_semaphoresRenderCompleted;

    // By default, enable V-Sync.
    // Can be toggled with the V key.
    Bool m_vSync              = true;
    Bool m_tearingSupported   = false;
    Bool m_isHoldingDearImgui = false;

    UInt m_currentBackBufferIndex = 0;

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

    // DearImGui
    VkDescriptorPool m_dearImGuiDescriptorPool = VK_NULL_HANDLE;
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
    void start_dearImGuiNewFrame() override;

    render::ISurface* get_newSurface() override;
};

}  // namespace m::vulkan

#endif  // M_RendererVulkanImpl