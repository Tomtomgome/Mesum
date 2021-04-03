#ifndef M_RendererVulkanImpl
#define M_RendererVulkanImpl
#pragma once

#include <MesumGraphics/Renderer.hpp>
#include <VulkanRendererCommon.hpp>

namespace m
{
namespace vulkan
{
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class VulkanSurface : public render::ISurface
{
   public:
    virtual ~VulkanSurface() = default;

    virtual void init_win32(render::Win32SurfaceInitData& a_data);
    virtual void init_x11(render::X11SurfaceInitData& a_data);

    virtual void init_dearImGui(Callback<void>& a_callback);

    virtual void render();
    virtual void resize(U32 a_width, U32 a_height);

    virtual void destroy();

   private:
    void init_internal();

    void destroy_swapChain();

   private:
    // The number of swap chain back buffers.
    static const U8 scm_numFrames = 3;

    VkSurfaceKHR             m_surface   = VK_NULL_HANDLE;
    VkSwapchainKHR           m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage>     m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;

    std::vector<U64>         m_tstpRenderFinish;
    std::vector<VkSemaphore> m_semaphoresImageAcquired;
    std::vector<VkSemaphore> m_semaphoresRenderCompleted;

    VkSemaphore m_timelineSemaphore;
    U64         m_timeline = 0;

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
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class VulkanRenderer : public render::IRenderer
{
   public:
    virtual void init();
    virtual void destroy();

    virtual void start_dearImGuiNewFrame();

    virtual render::ISurface* get_newSurface();
};

}  // namespace vulkan
}  // namespace m

#endif  // M_RendererVulkanImpl