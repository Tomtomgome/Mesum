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

    VkSemaphoreCreateInfo createSemaphore = {};
    createSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createSemaphore.flags = 0;

    VkSemaphoreTypeCreateInfo createSemaphoreType = {};
    createSemaphoreType.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    createSemaphoreType.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    createSemaphoreType.initialValue  = m_timeline;

    createSemaphore.pNext = &createSemaphoreType;

    if (vkCreateSemaphore(m_logicalDevice, &createSemaphore, nullptr,
                          &m_timelineSemaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create timeline semaphore");
    }

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

    vkDestroySemaphore(m_logicalDevice, m_timelineSemaphore, nullptr);

    vkDestroyDevice(m_logicalDevice, nullptr);

    destroy_debugMessenger(m_instance, m_debugUtil);

    vkDestroyInstance(m_instance, nullptr);
}

// sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
// sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
// sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
void VulkanContext::wait_onMainTimelineTstp(U64 a_tstpToWaitOn, U64 a_timeout)
{
    VkSemaphoreWaitInfo infoWaitSemaphore = {};
    infoWaitSemaphore.sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    infoWaitSemaphore.semaphoreCount = 1;
    infoWaitSemaphore.pSemaphores    = &gs_VulkanContexte->m_timelineSemaphore;
    infoWaitSemaphore.pValues        = &a_tstpToWaitOn;
    if (vkWaitSemaphores(VulkanContext::gs_VulkanContexte->get_logDevice(),
                         &infoWaitSemaphore, a_timeout) != VK_SUCCESS)
    {
        throw std::runtime_error("Wait on semaphore failled");
    }
}

// sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
// sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
// sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
U64 VulkanContext::submit_onMainTimeline(
    std::vector<VkSemaphore> const& a_semaphoresToWait,
    std::vector<VkSemaphore>        a_semaphoresToSignal)
{
    U64& finishValueOnTimeline = ++gs_VulkanContexte->m_timeline;

    std::vector<U64> signalValues;
    signalValues.resize(a_semaphoresToSignal.size() + 1, 0);
    signalValues[a_semaphoresToSignal.size()] = finishValueOnTimeline;

    std::vector<U64> dummyWaitValues;
    dummyWaitValues.resize(a_semaphoresToWait.size(), 0);

    VkTimelineSemaphoreSubmitInfo timelineInfo = {};
    timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    timelineInfo.waitSemaphoreValueCount   = dummyWaitValues.size();
    timelineInfo.pWaitSemaphoreValues      = dummyWaitValues.data();
    timelineInfo.signalSemaphoreValueCount = signalValues.size();
    timelineInfo.pSignalSemaphoreValues    = signalValues.data();

    a_semaphoresToSignal.push_back(gs_VulkanContexte->m_timelineSemaphore);

    VkSubmitInfo infoSubmit           = {};
    infoSubmit.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    infoSubmit.pNext                  = &timelineInfo;
    infoSubmit.waitSemaphoreCount     = a_semaphoresToWait.size();
    infoSubmit.pWaitSemaphores        = a_semaphoresToWait.data();
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
    infoSubmit.pWaitDstStageMask      = waitStages;
    infoSubmit.signalSemaphoreCount   = a_semaphoresToSignal.size();
    infoSubmit.pSignalSemaphores      = a_semaphoresToSignal.data();
    infoSubmit.commandBufferCount     = 0;
    infoSubmit.pCommandBuffers        = nullptr;
    if (vkQueueSubmit(VulkanContext::gs_VulkanContexte->get_graphicQueue(), 1,
                      &infoSubmit, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    return finishValueOnTimeline;
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