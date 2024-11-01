#include <MesumCore/Kernel/File.hpp>
#include <VulkanContext.hpp>

namespace m
{
namespace vulkan
{
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CommandQueue::init(mU32 const a_queueFamilyIndex, VkQueue a_queue)
{
    m_queueFamilyIndex = a_queueFamilyIndex;
    m_queue            = a_queue;

    VkSemaphoreCreateInfo createSemaphore = {};
    createSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createSemaphore.flags = 0;

    VkSemaphoreTypeCreateInfo createSemaphoreType = {};
    createSemaphoreType.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    createSemaphoreType.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    createSemaphoreType.initialValue  = m_timeline;

    createSemaphore.pNext = &createSemaphoreType;

    if (vkCreateSemaphore(VulkanContext::get_logDevice(), &createSemaphore,
                          nullptr, &m_timelineSemaphore) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create timeline semaphore");
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CommandQueue::destroy()
{
    vkDestroySemaphore(VulkanContext::get_logDevice(), m_timelineSemaphore,
                       nullptr);

    mAssert(m_inFlightCommandPools.empty());
    while (!m_freeCommandPools.empty())
    {
        CommandPoolEntry& entry = m_freeCommandPools.front();

        mAssert(entry.usedCommandBuffers.empty());
        while (!entry.availableCommandBuffers.empty())
        {
            vkFreeCommandBuffers(VulkanContext::get_logDevice(),
                                 entry.commandPool, 1,
                                 &entry.availableCommandBuffers.front());
            entry.availableCommandBuffers.pop();
        }

        vkDestroyCommandPool(VulkanContext::get_logDevice(), entry.commandPool,
                             nullptr);
        m_freeCommandPools.pop();
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CommandQueue::wait_onFenceValue(mU64 a_tstpToWaitOn, mU64 a_timeout)
{
    VkSemaphoreWaitInfo infoWaitSemaphore = {};
    infoWaitSemaphore.sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    infoWaitSemaphore.semaphoreCount = 1;
    infoWaitSemaphore.pSemaphores    = &m_timelineSemaphore;
    infoWaitSemaphore.pValues        = &a_tstpToWaitOn;
    if (vkWaitSemaphores(VulkanContext::get_logDevice(), &infoWaitSemaphore,
                         a_timeout) != VK_SUCCESS)
    {
        throw std::runtime_error("Wait on semaphore failled");
    }

    // Free/Reset associatedCommandPool;
    while (!m_inFlightCommandPools.empty() &&
           m_inFlightCommandPools.front().fenceValue <= a_tstpToWaitOn)
    {
        CommandPoolEntry& entry = m_inFlightCommandPools.front();
        check_vkResult(vkResetCommandPool(VulkanContext::get_logDevice(),
                                          entry.commandPool, 0));
        while (!entry.usedCommandBuffers.empty())
        {
            entry.availableCommandBuffers.push(
                entry.usedCommandBuffers.front());
            entry.usedCommandBuffers.pop();
        }

        m_freeCommandPools.push(m_inFlightCommandPools.front());
        m_inFlightCommandPools.pop();
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CommandQueue::push_waitOnSemaphores(
    std::vector<VkSemaphore> a_semaphoresToWait)
{
    uint64_t waitValue   = m_timeline;
    uint64_t signalValue = ++m_timeline;

    std::vector<mU64> dummyWaitValues;
    dummyWaitValues.resize(a_semaphoresToWait.size(), 0);
    dummyWaitValues.push_back(waitValue);

    VkTimelineSemaphoreSubmitInfo timelineInfo;
    timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    timelineInfo.pNext = NULL;
    timelineInfo.waitSemaphoreValueCount =
        static_cast<mU32>(dummyWaitValues.size());
    ;
    timelineInfo.pWaitSemaphoreValues      = dummyWaitValues.data();
    timelineInfo.signalSemaphoreValueCount = 1;
    timelineInfo.pSignalSemaphoreValues    = &signalValue;

    a_semaphoresToWait.push_back(m_timelineSemaphore);

    VkSubmitInfo infoSubmit = {};
    infoSubmit.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    infoSubmit.pNext        = &timelineInfo;
    infoSubmit.waitSemaphoreCount =
        static_cast<mU32>(a_semaphoresToWait.size());
    infoSubmit.pWaitSemaphores        = a_semaphoresToWait.data();
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
    infoSubmit.pWaitDstStageMask      = nullptr;  // waitStages;
    infoSubmit.signalSemaphoreCount   = 1;
    infoSubmit.pSignalSemaphores      = &m_timelineSemaphore;
    infoSubmit.commandBufferCount     = 0;
    infoSubmit.pCommandBuffers        = nullptr;
    if (vkQueueSubmit(m_queue, 1, &infoSubmit, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mU64 CommandQueue::signal_fence()
{
    uint64_t waitValue   = m_timeline;
    uint64_t signalValue = ++m_timeline;

    VkTimelineSemaphoreSubmitInfo timelineInfo = {};
    timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    timelineInfo.waitSemaphoreValueCount   = 1;
    timelineInfo.pWaitSemaphoreValues      = &waitValue;
    timelineInfo.signalSemaphoreValueCount = 1;
    timelineInfo.pSignalSemaphoreValues    = &signalValue;

    VkSubmitInfo infoSubmit         = {};
    infoSubmit.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    infoSubmit.pNext                = &timelineInfo;
    infoSubmit.waitSemaphoreCount   = 1;
    infoSubmit.pWaitSemaphores      = &m_timelineSemaphore;
    infoSubmit.pWaitDstStageMask    = nullptr;
    infoSubmit.signalSemaphoreCount = 1;
    infoSubmit.pSignalSemaphores    = &m_timelineSemaphore;
    infoSubmit.commandBufferCount   = 0;
    infoSubmit.pCommandBuffers      = nullptr;
    if (vkQueueSubmit(m_queue, 1, &infoSubmit, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    if (!m_freeCommandPools.empty())
    {
        m_freeCommandPools.front().fenceValue = signalValue;
        m_inFlightCommandPools.push(m_freeCommandPools.front());
        m_freeCommandPools.pop();
    }

    return signalValue;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CommandQueue::signal_semaphores(
    std::vector<VkSemaphore> a_semaphoresToSignal)
{
    uint64_t waitValue   = m_timeline;
    uint64_t signalValue = ++m_timeline;

    std::vector<mU64> signalValues;
    signalValues.resize(a_semaphoresToSignal.size() + 1, 0);
    signalValues[a_semaphoresToSignal.size()] = signalValue;

    VkTimelineSemaphoreSubmitInfo timelineInfo;
    timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    timelineInfo.pNext = NULL;
    timelineInfo.waitSemaphoreValueCount = 1;
    timelineInfo.pWaitSemaphoreValues    = &waitValue;
    timelineInfo.signalSemaphoreValueCount =
        static_cast<mU32>(signalValues.size());
    ;
    timelineInfo.pSignalSemaphoreValues = signalValues.data();

    a_semaphoresToSignal.push_back(m_timelineSemaphore);

    VkSubmitInfo infoSubmit       = {};
    infoSubmit.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    infoSubmit.pNext              = &timelineInfo;
    infoSubmit.waitSemaphoreCount = 1;
    infoSubmit.pWaitSemaphores    = &m_timelineSemaphore;
    infoSubmit.pWaitDstStageMask  = nullptr;
    infoSubmit.signalSemaphoreCount =
        static_cast<mU32>(a_semaphoresToSignal.size());
    infoSubmit.pSignalSemaphores  = a_semaphoresToSignal.data();
    infoSubmit.commandBufferCount = 0;
    infoSubmit.pCommandBuffers    = nullptr;
    if (vkQueueSubmit(m_queue, 1, &infoSubmit, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CommandQueue::flush()
{
    uint64_t fenceValueForSignal = signal_fence();
    wait_onFenceValue(fenceValueForSignal);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
VkCommandBuffer CommandQueue::get_commandBuffer()
{
    VkCommandBuffer   commandBuffer;
    CommandPoolEntry* pCommandPoolEntry = nullptr;
    if (!m_freeCommandPools.empty())
    {
        pCommandPoolEntry = &m_freeCommandPools.front();
    }
    else
    {
        CommandPoolEntry createdCommandPull;
        createdCommandPull.commandPool = create_commandPool();
        m_freeCommandPools.push(createdCommandPull);
        pCommandPoolEntry = &m_freeCommandPools.front();
    }

    CommandPoolEntry& commandPoolEntry = *pCommandPoolEntry;

    if (!commandPoolEntry.availableCommandBuffers.empty())
    {
        commandBuffer = commandPoolEntry.availableCommandBuffers.front();
        commandPoolEntry.availableCommandBuffers.pop();
    }
    else
    {
        commandBuffer = create_commandBuffer(commandPoolEntry.commandPool);
    }
    commandPoolEntry.usedCommandBuffers.push(commandBuffer);

    VkCommandBufferBeginInfo info = {};
    info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    check_vkResult(vkBeginCommandBuffer(commandBuffer, &info));

    return commandBuffer;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CommandQueue::submit_commandBuffer(VkCommandBuffer a_commandBuffer)
{
    check_vkResult(vkEndCommandBuffer(a_commandBuffer));

    uint64_t waitValue   = m_timeline;
    uint64_t signalValue = ++m_timeline;

    VkTimelineSemaphoreSubmitInfo timelineInfo;
    timelineInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    timelineInfo.pNext = NULL;
    timelineInfo.waitSemaphoreValueCount   = 1;
    timelineInfo.pWaitSemaphoreValues      = &waitValue;
    timelineInfo.signalSemaphoreValueCount = 1;
    timelineInfo.pSignalSemaphoreValues    = &signalValue;

    VkSubmitInfo infoSubmit           = {};
    infoSubmit.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    infoSubmit.pNext                  = &timelineInfo;
    infoSubmit.waitSemaphoreCount     = 1;
    infoSubmit.pWaitSemaphores        = &m_timelineSemaphore;
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
    infoSubmit.pWaitDstStageMask      = waitStages;
    infoSubmit.signalSemaphoreCount   = 1;
    infoSubmit.pSignalSemaphores      = &m_timelineSemaphore;
    infoSubmit.commandBufferCount     = 1;
    infoSubmit.pCommandBuffers        = &a_commandBuffer;
    if (vkQueueSubmit(m_queue, 1, &infoSubmit, VK_NULL_HANDLE) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
VkCommandPool CommandQueue::create_commandPool()
{
    VkCommandPool           commandPool;
    VkCommandPoolCreateInfo createCommandPoolInfo = {};
    createCommandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createCommandPoolInfo.queueFamilyIndex = m_queueFamilyIndex;

    check_vkResult(vkCreateCommandPool(VulkanContext::get_logDevice(),
                                       &createCommandPoolInfo, nullptr,
                                       &commandPool));

    return commandPool;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
VkCommandBuffer CommandQueue::create_commandBuffer(VkCommandPool a_pool)
{
    VkCommandBuffer             commandBuffer;
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = a_pool;
    allocInfo.commandBufferCount = 1;

    check_vkResult(vkAllocateCommandBuffers(VulkanContext::get_logDevice(),
                                            &allocInfo, &commandBuffer));

    return commandBuffer;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
VulkanContext* VulkanContext::gs_VulkanContexte;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanContext::init()
{
    create_instance(m_instance);

    setup_debugMessenger(m_instance, m_debugUtil);

    select_physicalDevice(m_instance, m_physicalDevice);

    mU32    queueFamilyIndex;
    VkQueue queue = VK_NULL_HANDLE;
    create_logicalDevice(m_physicalDevice, m_logicalDevice, queue,
                         queueFamilyIndex);
    m_queue.init(queueFamilyIndex, queue);

    create_vmaAllocator(m_instance, m_physicalDevice, m_logicalDevice,
                        m_allocator);

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
    m_queue.destroy();

    vkDestroyCommandPool(m_logicalDevice, m_utilityCommandPool, nullptr);

    vkDestroyDevice(m_logicalDevice, nullptr);

    destroy_debugMessenger(m_instance, m_debugUtil);

    vkDestroyInstance(m_instance, nullptr);
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

    vkQueueSubmit(m_queue.get_queue(), 1, &infoSubmit, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_queue.get_queue());

    vkFreeCommandBuffers(m_logicalDevice, m_utilityCommandPool, 1,
                         &a_commandBuffer);
}
}  // namespace vulkan
}  // namespace m