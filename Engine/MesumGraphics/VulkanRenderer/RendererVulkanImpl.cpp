#include <imgui_impl_vulkan.h>

#include <RendererVulkanImpl.hpp>
#include <VulkanContext.hpp>
#include <limits>

namespace m
{
namespace vulkan
{
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanSurface::init_win32(render::Win32SurfaceInitData& a_data)
{
#ifdef M_WIN32
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd      = a_data.m_hwnd;
    createInfo.hinstance = GetModuleHandle(nullptr);

    if (vkCreateWin32SurfaceKHR(
            VulkanContext::gs_VulkanContexte->get_instance(), &createInfo,
            nullptr, &m_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
    m_clientWidth  = a_data.m_width;
    m_clientHeight = a_data.m_height;
#else
    // Wrong platform
    mInterrupt
#endif
    init_internal();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanSurface::init_x11(render::X11SurfaceInitData& a_data)
{
#ifdef M_UNIX
    m_clientWidth  = a_data.m_width;
    m_clientHeight = a_data.m_height;
#else
    // Wrong platform
    mInterrupt
#endif
    init_internal();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanSurface::init_dearImGui(Callback<void>& a_callback) {}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanSurface::render()
{
    // Check oldRenderHasFinished for next frame

    VkSemaphoreWaitInfo infoWaitSemaphore = {};
    infoWaitSemaphore.sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    infoWaitSemaphore.semaphoreCount = 1;
    infoWaitSemaphore.pSemaphores    = &m_timelineSemaphore;
    infoWaitSemaphore.pValues = &m_tstpRenderFinish[m_currentBackBufferIndex];
    if (vkWaitSemaphores(VulkanContext::gs_VulkanContexte->get_logDevice(),
                         &infoWaitSemaphore,
                         std::numeric_limits<U64>::max()) != VK_SUCCESS)
    {
        throw std::runtime_error("Wait on semaphore failled");
    }
    // Aquire next image
    UInt imageIndex;
    vkAcquireNextImageKHR(VulkanContext::gs_VulkanContexte->get_logDevice(),
                          m_swapChain, std::numeric_limits<U64>::max(),
                          m_semaphoresImageAcquired[m_currentBackBufferIndex],
                          VK_NULL_HANDLE, &imageIndex);
    m_tstpRenderFinish[m_currentBackBufferIndex] = ++m_timeline;

    // Submission
    const uint64_t signalSemaphoreValues[2] = {
        m_timeline,  // value for "timeline"
        0            // ignored, binary semaphore.
    };

    U64 dummyWait;

    VkTimelineSemaphoreSubmitInfo timelineInfo = {};
    timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    timelineInfo.waitSemaphoreValueCount   = 1;
    timelineInfo.pWaitSemaphoreValues      = &dummyWait;
    timelineInfo.signalSemaphoreValueCount = 2;
    timelineInfo.pSignalSemaphoreValues    = signalSemaphoreValues;

    const VkSemaphore signalSemaphores[2] = {
        m_timelineSemaphore,  // Track timeline semaphore work completion
        m_semaphoresRenderCompleted[m_currentBackBufferIndex]  // Unblock
                                                               // presentation
    };

    VkSubmitInfo infoSubmit           = {};
    infoSubmit.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    infoSubmit.pNext                  = &timelineInfo;
    infoSubmit.waitSemaphoreCount     = 1;
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
    infoSubmit.pWaitSemaphores =
        &m_semaphoresImageAcquired[m_currentBackBufferIndex];
    infoSubmit.pWaitDstStageMask    = waitStages;
    infoSubmit.signalSemaphoreCount = 2;
    infoSubmit.pSignalSemaphores    = signalSemaphores;
    infoSubmit.commandBufferCount   = 0;
    infoSubmit.pCommandBuffers      = nullptr;
    if (vkQueueSubmit(VulkanContext::gs_VulkanContexte->get_graphicQueue(), 1,
                      &infoSubmit, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // Presentation
    VkPresentInfoKHR infoPresent   = {};
    infoPresent.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    infoPresent.waitSemaphoreCount = 1;
    infoPresent.pWaitSemaphores =
        &m_semaphoresRenderCompleted[m_currentBackBufferIndex];
    infoPresent.swapchainCount = 1;
    infoPresent.pSwapchains    = &m_swapChain;
    infoPresent.pImageIndices  = &imageIndex;
    vkQueuePresentKHR(VulkanContext::gs_VulkanContexte->get_presentationQueue(),
                      &infoPresent);

    m_currentBackBufferIndex = (m_currentBackBufferIndex + 1) % scm_numFrames;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanSurface::resize(U32 a_width, U32 a_height)
{
    if (m_clientWidth != a_width || m_clientHeight != a_height)
    {
        // Don't allow 0 size swap chain back buffers.
        m_clientWidth  = std::max(1u, a_width);
        m_clientHeight = std::max(1u, a_height);

        destroy_swapChain();
        init_swapChain();
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanSurface::destroy()
{
    destroy_swapChain();

    for (size_t i = 0; i < scm_numFrames; i++)
    {
        vkDestroySemaphore(VulkanContext::gs_VulkanContexte->get_logDevice(),
                           m_semaphoresImageAcquired[i], nullptr);
        vkDestroySemaphore(VulkanContext::gs_VulkanContexte->get_logDevice(),
                           m_semaphoresRenderCompleted[i], nullptr);
    }

    vkDestroySemaphore(VulkanContext::gs_VulkanContexte->get_logDevice(),
                       m_timelineSemaphore, nullptr);

    vkDestroySurfaceKHR(VulkanContext::gs_VulkanContexte->get_instance(),
                        m_surface, nullptr);
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
void VulkanSurface::init_internal()
{
    U32 queueFamilyIndex;
    find_graphicQueueFamilyIndex(
        VulkanContext::gs_VulkanContexte->get_physDevice(), queueFamilyIndex);

    VkBool32 surfaceSupported;
    vkGetPhysicalDeviceSurfaceSupportKHR(
        VulkanContext::gs_VulkanContexte->get_physDevice(), queueFamilyIndex,
        m_surface, &surfaceSupported);
    if (!surfaceSupported)
    {
        throw std::runtime_error(
            "Surfaces should be supported by the application");
    }

    std::vector<VkSurfaceFormatKHR> supportedFormats;

    U32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        VulkanContext::gs_VulkanContexte->get_physDevice(), m_surface,
        &formatCount, nullptr);

    if (formatCount == 0)
    {
        throw std::runtime_error("Need to support at least some formats");
    }

    supportedFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        VulkanContext::gs_VulkanContexte->get_physDevice(), m_surface,
        &formatCount, supportedFormats.data());

    Bool isFormatSupported = false;
    for (size_t i = 0; i < formatCount; i++)
    {
        if (supportedFormats[i].format == scm_selectedSwapChainFormat)
        {
            isFormatSupported = true;
            break;
        }
    }

    if (!isFormatSupported)
    {
        throw std::runtime_error(
            "VK_FORMAT_B8G8R8A8_UNORM Needs to be supported at the moment");
    }

    VkSemaphoreCreateInfo createSemaphore = {};
    createSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createSemaphore.flags = 0;

    VkSemaphoreTypeCreateInfo createSemaphoreType = {};
    createSemaphoreType.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    createSemaphoreType.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    createSemaphoreType.initialValue  = m_timeline;

    createSemaphore.pNext = &createSemaphoreType;

    if (vkCreateSemaphore(VulkanContext::gs_VulkanContexte->get_logDevice(),
                          &createSemaphore, nullptr,
                          &m_timelineSemaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create timeline semaphore");
    }

    m_tstpRenderFinish.resize(scm_numFrames, 0);

    m_semaphoresImageAcquired.resize(scm_numFrames);
    m_semaphoresRenderCompleted.resize(scm_numFrames);

    createSemaphore       = {};
    createSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < scm_numFrames; i++)
    {
        if (vkCreateSemaphore(VulkanContext::gs_VulkanContexte->get_logDevice(),
                              &createSemaphore, nullptr,
                              &m_semaphoresImageAcquired[i]) != VK_SUCCESS ||
            vkCreateSemaphore(VulkanContext::gs_VulkanContexte->get_logDevice(),
                              &createSemaphore, nullptr,
                              &m_semaphoresRenderCompleted[i]) != VK_SUCCESS)
        {
            throw std::runtime_error(
                "failed to create semaphores for a frame!");
        }
    }

    init_swapChain();
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
void VulkanSurface::init_swapChain()
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        VulkanContext::gs_VulkanContexte->get_physDevice(), m_surface,
        &surfaceCapabilities);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = m_surface;
    createInfo.minImageCount    = scm_numFrames;
    createInfo.imageFormat      = scm_selectedSwapChainFormat;
    createInfo.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent      = {m_clientWidth, m_clientHeight};
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform     = surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped          = VK_TRUE;
    createInfo.oldSwapchain     = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(VulkanContext::gs_VulkanContexte->get_logDevice(),
                             &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    U32 imageCount;
    vkGetSwapchainImagesKHR(VulkanContext::gs_VulkanContexte->get_logDevice(),
                            m_swapChain, &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(VulkanContext::gs_VulkanContexte->get_logDevice(),
                            m_swapChain, &imageCount, m_swapChainImages.data());

    m_swapChainImageViews.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image        = m_swapChainImages[i];
        createInfo.viewType     = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format       = scm_selectedSwapChainFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount     = 1;
        if (vkCreateImageView(VulkanContext::gs_VulkanContexte->get_logDevice(),
                              &createInfo, nullptr,
                              &m_swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
void VulkanSurface::destroy_swapChain()
{
    VkSemaphoreWaitInfo infoWaitSemaphore = {};
    infoWaitSemaphore.sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    infoWaitSemaphore.semaphoreCount = 1;
    infoWaitSemaphore.pSemaphores    = &m_timelineSemaphore;
    infoWaitSemaphore.pValues        = &m_timeline;
    if (vkWaitSemaphores(VulkanContext::gs_VulkanContexte->get_logDevice(),
                         &infoWaitSemaphore,
                         std::numeric_limits<U64>::max()) != VK_SUCCESS)
    {
        throw std::runtime_error("Wait on semaphore failled");
    }

    for (Int i = 0; i < scm_numFrames; ++i)
    {
        m_tstpRenderFinish[i] = m_timeline;
    }

    for (auto imageView : m_swapChainImageViews)
    {
        vkDestroyImageView(VulkanContext::gs_VulkanContexte->get_logDevice(),
                           imageView, nullptr);
    }

    vkDestroySwapchainKHR(VulkanContext::gs_VulkanContexte->get_logDevice(),
                          m_swapChain, nullptr);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanRenderer::init()
{
    VulkanContext::gs_VulkanContexte = new VulkanContext();
    VulkanContext::gs_VulkanContexte->init();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanRenderer::destroy()
{
    VulkanContext::gs_VulkanContexte->deinit();
    delete VulkanContext::gs_VulkanContexte;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanRenderer::start_dearImGuiNewFrame()
{
    ImGui_ImplVulkan_NewFrame();
    // VulkanContext::gs_VulkanContexte->m_dearImGuiPlatImplCallback.call();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
render::ISurface* VulkanRenderer::get_newSurface()
{
    return new VulkanSurface();
}
}  // namespace vulkan
}  // namespace m