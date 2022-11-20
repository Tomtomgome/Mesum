#include "RendererVulkanImpl.hpp"

namespace m::vulkan
{
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSwapchain::init_win32(Desc const& a_desc, DescWin32 const& a_descWin32)
{
#ifndef M_WIN32
    // Wrong platform
    mInterrupt;
#else
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd      = a_descWin32.hwd;
    createInfo.hinstance = GetModuleHandle(nullptr);

    if (vkCreateWin32SurfaceKHR(VulkanContext::get_instance(), &createInfo,
                                nullptr, &m_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }

#endif
    _init(a_desc);

    m_semaphoresImageAcquired.resize(a_desc.bufferCount);
    m_semaphoresRenderCompleted.resize(a_desc.bufferCount);

    VkSemaphoreCreateInfo createSemaphore = {};
    createSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createSemaphore.flags = 0;

    for (size_t i = 0; i < a_desc.bufferCount; i++)
    {
        if (vkCreateSemaphore(VulkanContext::get_logDevice(), &createSemaphore,
                              nullptr,
                              &m_semaphoresImageAcquired[i]) != VK_SUCCESS ||
            vkCreateSemaphore(VulkanContext::get_logDevice(), &createSemaphore,
                              nullptr,
                              &m_semaphoresRenderCompleted[i]) != VK_SUCCESS)
        {
            throw std::runtime_error(
                "failed to create semaphores for a frame!");
        }
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSwapchain::init_x11(Desc const& a_config, Descx11 const& a_data)
{
    mNotImplemented
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSwapchain::destroy()
{
    _destroy_swapChain();

    VulkanContext::get_commandQueue().flush();

    mAssert(m_semaphoresImageAcquired.size() ==
            m_semaphoresRenderCompleted.size());
    for (size_t i = 0; i < m_semaphoresImageAcquired.size(); i++)
    {
        vkDestroySemaphore(VulkanContext::get_logDevice(),
                           m_semaphoresImageAcquired[i], nullptr);
        vkDestroySemaphore(VulkanContext::get_logDevice(),
                           m_semaphoresRenderCompleted[i], nullptr);
    }

    vkDestroySurfaceKHR(VulkanContext::get_instance(), m_surface, nullptr);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSwapchain::resize(mU32 a_width, mU32 a_height)
{
    if (m_currentDesc.width != a_width || m_currentDesc.height != a_height)
    {
        m_currentDesc.width  = a_width;
        m_currentDesc.height = a_height;
        _destroy_swapChain();
        _init_swapChain();
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSwapchain::acquire_image(mUInt a_waitIndex)
{
    vkAcquireNextImageKHR(vulkan::VulkanContext::get_logDevice(), m_swapChain,
                          std::numeric_limits<mU64>::max(),
                          m_semaphoresImageAcquired[a_waitIndex],
                          VK_NULL_HANDLE, &m_currentImageIndex);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSwapchain::present(mUInt a_presentIndex)
{
    VkPresentInfoKHR infoPresent   = {};
    infoPresent.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    infoPresent.waitSemaphoreCount = 1;
    infoPresent.pWaitSemaphores = &m_semaphoresRenderCompleted[a_presentIndex];
    infoPresent.swapchainCount  = 1;
    infoPresent.pSwapchains     = &m_swapChain;
    infoPresent.pImageIndices   = &m_currentImageIndex;
    vkQueuePresentKHR(vulkan::VulkanContext::get_commandQueue().get_queue(),
                      &infoPresent);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mRenderTarget mSwapchain::get_currentRenderTarget()
{
    mRenderTarget renderTarget{};
    renderTarget.imageView = m_swapChainImageViews[m_currentImageIndex];
    renderTarget.width = m_currentDesc.width;
    renderTarget.height = m_currentDesc.height;
    return renderTarget;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void mSwapchain::_init(Desc const& a_desc)
{
    m_currentDesc = a_desc;
    mU32 queueFamilyIndex;
    find_graphicQueueFamilyIndex(VulkanContext::get_physDevice(),
                                 queueFamilyIndex);

    VkBool32 surfaceSupported;
    vkGetPhysicalDeviceSurfaceSupportKHR(VulkanContext::get_physDevice(),
                                         queueFamilyIndex, m_surface,
                                         &surfaceSupported);
    if (!surfaceSupported)
    {
        throw std::runtime_error(
            "Surfaces should be supported by the application");
    }

    std::vector<VkSurfaceFormatKHR> supportedFormats;

    mU32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanContext::get_physDevice(),
                                         m_surface, &formatCount, nullptr);

    supportedFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanContext::get_physDevice(),
                                         m_surface, &formatCount,
                                         supportedFormats.data());

    mBool isFormatSupported = false;
    for (size_t i = 0; i < formatCount; i++)
    {
        if (supportedFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM)
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

    _init_swapChain();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void mSwapchain::_init_swapChain()
{
    VkFormat                 swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanContext::get_physDevice(),
                                              m_surface, &surfaceCapabilities);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = m_surface;
    createInfo.minImageCount    = m_currentDesc.bufferCount;
    createInfo.imageFormat      = swapchainFormat;
    createInfo.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent      = {m_currentDesc.width, m_currentDesc.height};
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform     = surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped          = VK_TRUE;
    createInfo.oldSwapchain     = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(VulkanContext::get_logDevice(), &createInfo,
                             nullptr, &m_swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    mU32 imageCount;
    vkGetSwapchainImagesKHR(VulkanContext::get_logDevice(), m_swapChain,
                            &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(VulkanContext::get_logDevice(), m_swapChain,
                            &imageCount, m_swapChainImages.data());

    m_swapChainImageViews.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++)
    {
        VkImageViewCreateInfo createInfoImageViews{};
        createInfoImageViews.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfoImageViews.image = m_swapChainImages[i];
        createInfoImageViews.viewType     = VK_IMAGE_VIEW_TYPE_2D;
        createInfoImageViews.format       = swapchainFormat;
        createInfoImageViews.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfoImageViews.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfoImageViews.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfoImageViews.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfoImageViews.subresourceRange.aspectMask =
            VK_IMAGE_ASPECT_COLOR_BIT;
        createInfoImageViews.subresourceRange.baseMipLevel   = 0;
        createInfoImageViews.subresourceRange.levelCount     = 1;
        createInfoImageViews.subresourceRange.baseArrayLayer = 0;
        createInfoImageViews.subresourceRange.layerCount     = 1;
        if (vkCreateImageView(VulkanContext::get_logDevice(),
                              &createInfoImageViews, nullptr,
                              &m_swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }

        VkCommandBuffer commandBuffer =
            VulkanContext::gs_VulkanContexte->get_singleUseCommandBuffer();

        VkImageMemoryBarrier barrier{};
        barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image               = m_swapChainImages[i];
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                             VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0, nullptr,
                             0, nullptr, 1, &barrier);

        VulkanContext::gs_VulkanContexte->submit_singleUseCommandBuffer(
            commandBuffer);
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void mSwapchain::_destroy_swapChain()
{
    for (auto imageView : m_swapChainImageViews)
    {
        vkDestroyImageView(VulkanContext::get_logDevice(), imageView, nullptr);
    }

    vkDestroySwapchainKHR(VulkanContext::get_logDevice(), m_swapChain, nullptr);
}

}  // namespace m::vulkan