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
extern MesumGraphicsApi const logging::ChannelID VK_RENDERER_ID;

void create_instance(VkInstance& a_InstaceToCreate);
void setup_debugUtilsMessengerCreateInfoExt(
    VkDebugUtilsMessengerCreateInfoEXT a_createInfo);
void setup_debugMessenger(VkInstance                a_instance,
                          VkDebugUtilsMessengerEXT& a_debugUtil);
void destroy_debugMessenger(VkInstance                a_instance,
                            VkDebugUtilsMessengerEXT& a_debugUtil);

void select_physicalDevice(VkInstance        a_instance,
                           VkPhysicalDevice& a_physicalDevice);

void create_logicalDevice(VkPhysicalDevice a_physicalDevice,
                          VkDevice& a_logicalDevice, VkQueue& a_queue);

};  // namespace vulkan
};  // namespace m

#endif  // M_VulkanRendererCommon