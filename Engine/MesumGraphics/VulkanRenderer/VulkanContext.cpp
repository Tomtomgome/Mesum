#include <VulkanContext.hpp>

namespace m
{
namespace vulkan
{
VulkanContext* VulkanContext::gs_VulkanContexte;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanContext::init()
{
    create_instance(m_instance);

    setup_debugMessenger(m_instance, m_debugUtil);

    select_physicalDevice(m_instance, m_physicalDevice);

    create_logicalDevice(m_physicalDevice, m_logicalDevice, m_queue);

    U32 queueFamilyIndex;
    find_graphicQueueFamilyIndex(m_physicalDevice, queueFamilyIndex);

    VkCommandPoolCreateInfo createCommandPool = {};
    createCommandPool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createCommandPool.queueFamilyIndex = queueFamilyIndex;

    if (vkCreateCommandPool(m_logicalDevice, &createCommandPool, nullptr,
                            &m_utilityCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanContext::deinit()
{
    vkDestroyCommandPool(m_logicalDevice, m_utilityCommandPool, nullptr);

    vkDestroyDevice(m_logicalDevice, nullptr);

    destroy_debugMessenger(m_instance, m_debugUtil);

    vkDestroyInstance(m_instance, nullptr);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
VkCommandBuffer VulkanContext::get_singleUseCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_utilityCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanContext::submit_signleUseCommandBuffer(
    VkCommandBuffer a_commandBuffer)
{
    vkEndCommandBuffer(a_commandBuffer);

    VkSubmitInfo infoSubmit       = {};
    infoSubmit.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    infoSubmit.commandBufferCount = 1;
    infoSubmit.pCommandBuffers    = &a_commandBuffer;

    vkQueueSubmit(m_queue, 1, &infoSubmit, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_queue);

    vkFreeCommandBuffers(m_logicalDevice, m_utilityCommandPool, 1,
                         &a_commandBuffer);
}
}  // namespace vulkan
}  // namespace m