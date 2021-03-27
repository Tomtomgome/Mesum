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
#else
    // Wrong platform
    mInterrupt
#endif
}

void VulkanSurface::init_x11(render::X11SurfaceInitData& a_data)
{
#ifdef M_UNIX

#else
    // Wrong platform
    mInterrupt
#endif
}

void VulkanSurface::init_dearImGui(Callback<void>& a_callback) {}

void VulkanSurface::render() {}
void VulkanSurface::resize(U32 a_width, U32 a_height) {}

void VulkanSurface::destroy()
{
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