#include <MesumCore/Kernel/File.hpp>
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

    create_logicalDevice(m_physicalDevice, m_logicalDevice, m_queue,
                         m_queueFamilyIndex);

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

    mU32 queueFamilyIndex;
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
void VulkanContext::wait_onMainTimelineTstp(mU64 a_tstpToWaitOn, mU64 a_timeout)
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
mU64 VulkanContext::submit_onMainTimeline(
    VkCommandBuffer const&          a_commandBuffer,
    std::vector<VkSemaphore> const& a_semaphoresToWait,
    std::vector<VkSemaphore>        a_semaphoresToSignal)
{
    mU64& finishValueOnTimeline = ++gs_VulkanContexte->m_timeline;

    std::vector<mU64> signalValues;
    signalValues.resize(a_semaphoresToSignal.size() + 1, 0);
    signalValues[a_semaphoresToSignal.size()] = finishValueOnTimeline;

    std::vector<mU64> dummyWaitValues;
    dummyWaitValues.resize(a_semaphoresToWait.size(), 0);

    VkTimelineSemaphoreSubmitInfo timelineInfo = {};
    timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    timelineInfo.waitSemaphoreValueCount =
        static_cast<mU32>(dummyWaitValues.size());
    timelineInfo.pWaitSemaphoreValues = dummyWaitValues.data();
    timelineInfo.signalSemaphoreValueCount =
        static_cast<mU32>(signalValues.size());
    timelineInfo.pSignalSemaphoreValues = signalValues.data();

    a_semaphoresToSignal.push_back(gs_VulkanContexte->m_timelineSemaphore);

    VkSubmitInfo infoSubmit       = {};
    infoSubmit.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    infoSubmit.pNext              = &timelineInfo;
    infoSubmit.waitSemaphoreCount = static_cast<mU32>(a_semaphoresToWait.size());
    infoSubmit.pWaitSemaphores    = a_semaphoresToWait.data();
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
    infoSubmit.pWaitDstStageMask      = waitStages;
    infoSubmit.signalSemaphoreCount =
        static_cast<mU32>(a_semaphoresToSignal.size());
    infoSubmit.pSignalSemaphores  = a_semaphoresToSignal.data();
    infoSubmit.commandBufferCount = 1;
    infoSubmit.pCommandBuffers    = &a_commandBuffer;
    if (vkQueueSubmit(VulkanContext::gs_VulkanContexte->get_graphicQueue(), 1,
                      &infoSubmit, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    return finishValueOnTimeline;
}

// sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
// sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
// sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
void VulkanContext::present(VkPresentInfoKHR const& a_infoPresent)
{
    vkQueuePresentKHR(VulkanContext::get_presentationQueue(), &a_infoPresent);
}

// sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
// sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
// sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
mU32 VulkanContext::get_memoryTypeIndex(mU32                  a_typeFilter,
                                       VkMemoryPropertyFlags a_properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(gs_VulkanContexte->get_physDevice(),
                                        &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((a_typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & a_properties) ==
                a_properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

// sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
// sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
// sssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss
VkShaderModule VulkanContext::create_shaderModule(
    std::string const& a_shaderPath)
{
    std::vector<char> binary;
    files::copy_fileToBinary(a_shaderPath, binary);

    mAssert(a_shaderPath.size() > 0);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = binary.size();
    createInfo.pCode    = reinterpret_cast<const uint32_t*>(binary.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(gs_VulkanContexte->get_logDevice(), &createInfo,
                             nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }
    return shaderModule;
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
void VulkanContext::submit_singleUseCommandBuffer(
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