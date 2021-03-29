#include <imgui_impl_vulkan.h>

#include <RendererVulkanImpl.hpp>
#include <VulkanContext.hpp>

namespace m
{
namespace vulkan
{
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
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

    VkFormat selectedSwapChainFormat = VK_FORMAT_B8G8R8A8_UNORM;
    Bool     isFormatSupported       = false;
    for (size_t i = 0; i < formatCount; i++)
    {
        if (supportedFormats[i].format == selectedSwapChainFormat)
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

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        VulkanContext::gs_VulkanContexte->get_physDevice(), m_surface,
        &surfaceCapabilities);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = m_surface;
    createInfo.minImageCount    = scm_numFrames;
    createInfo.imageFormat      = selectedSwapChainFormat;
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
        createInfo.format       = selectedSwapChainFormat;
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

void VulkanSurface::init_dearImGui(Callback<void>& a_callback) {}

void VulkanSurface::render() {}
void VulkanSurface::resize(U32 a_width, U32 a_height) {}

void VulkanSurface::destroy()
{
    for (auto imageView : m_swapChainImageViews)
    {
        vkDestroyImageView(VulkanContext::gs_VulkanContexte->get_logDevice(),
                           imageView, nullptr);
    }

    vkDestroySwapchainKHR(VulkanContext::gs_VulkanContexte->get_logDevice(),
                          m_swapChain, nullptr);
    vkDestroySurfaceKHR(VulkanContext::gs_VulkanContexte->get_instance(),
                        m_surface, nullptr);
}
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
void VulkanRenderer::init()
{
    VulkanContext::gs_VulkanContexte = new VulkanContext();
    VulkanContext::gs_VulkanContexte->init();
}

void VulkanRenderer::destroy()
{
    VulkanContext::gs_VulkanContexte->deinit();
    delete VulkanContext::gs_VulkanContexte;
}

void VulkanRenderer::start_dearImGuiNewFrame()
{
    ImGui_ImplVulkan_NewFrame();
    // VulkanContext::gs_VulkanContexte->m_dearImGuiPlatImplCallback.call();
}

render::ISurface* VulkanRenderer::get_newSurface()
{
    return new VulkanSurface();
}
}  // namespace vulkan
}  // namespace m