#include <VulkanContext.hpp>

namespace m
{
namespace vulkan
{
VulkanContext* VulkanContext::gs_VulkanContexte;

void VulkanContext::init() {
    create_instance(m_instance);

    setup_debugMessenger(m_instance, m_debugUtil);

    select_physicalDevice(m_instance, m_physicalDevice);

    create_logicalDevice(m_physicalDevice, m_logicalDevice, m_queue);
}

void VulkanContext::deinit() {
    vkDestroyDevice(m_logicalDevice, nullptr);

    destroy_debugMessenger(m_instance, m_debugUtil);

    vkDestroyInstance(m_instance, nullptr);
}
}  // namespace vulkan
}  // namespace m