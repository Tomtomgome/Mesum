#ifndef M_VulkanRendererCommon
#define M_VulkanRendererCommon
#pragma once

#include <vulkan/vulkan.h>

#include <MesumCore/Kernel/Asserts.hpp>
#include <MesumGraphics/Common.hpp>

namespace m
{
namespace vulkan
{
extern MesumGraphicsApi const logging::ChannelID VK_RENDERER_ID;

void create_instance(VkInstance& a_InstaceToCreate);
void setup_debugMessenger(VkInstance const&         a_instance,
                          VkDebugUtilsMessengerEXT& a_debugUtil);
};  // namespace vulkan
};  // namespace m

#endif  // M_VulkanRendererCommon