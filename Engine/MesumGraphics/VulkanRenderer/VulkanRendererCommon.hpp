#ifndef M_VulkanRendererCommon
#define M_VulkanRendererCommon
#pragma once

#include <MesumCore/Kernel/Asserts.hpp>
#include <MesumGraphics/Common.hpp>

#if defined M_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined M_UNIX
#define VK_USE_PLATFORM_XCB_KHR
#endif
#include <vulkan/vulkan.h>

namespace m
{
namespace vulkan
{
extern MesumGraphicsApi const logging::mChannelID VK_RENDERER_ID;

inline void check_vkResult(VkResult err)
{
    if (err == 0)
    {
        return;
    }
    mLog_errorTo(VK_RENDERER_ID, "[vulkan] Error: VkResult = ", err);
    if (err < 0)
    {
        abort();
    }
}

void create_instance(VkInstance& a_InstaceToCreate);
void setup_debugUtilsMessengerCreateInfoExt(
    VkDebugUtilsMessengerCreateInfoEXT& a_createInfo);
void setup_debugMessenger(VkInstance                a_instance,
                          VkDebugUtilsMessengerEXT& a_debugUtil);
void destroy_debugMessenger(VkInstance                a_instance,
                            VkDebugUtilsMessengerEXT& a_debugUtil);

void select_physicalDevice(VkInstance        a_instance,
                           VkPhysicalDevice& a_physicalDevice);

mBool find_graphicQueueFamilyIndex(VkPhysicalDevice a_physicalDevice,
                                  mU32&             a_queueFamilyIndex);
void create_logicalDevice(VkPhysicalDevice a_physicalDevice,
                          VkDevice& a_logicalDevice, VkQueue& a_queue,
                          mU32& a_queueFamilyIndex);

};  // namespace vulkan
};  // namespace m

#endif  // M_VulkanRendererCommon