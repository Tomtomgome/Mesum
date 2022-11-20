#ifndef M_VulkanContext
#define M_VulkanContext

#include <MesumCore/Kernel/Callbacks.hpp>
#include <VulkanRendererCommon.hpp>

#include <queue>

namespace m
{
namespace vulkan
{
class CommandQueue
{
   public:
    void init(mU32 const a_queueFamilyIndex, VkQueue a_queue);
    void destroy();

    void wait_onFenceValue(mU64 a_tstpToWaitOn,
                           mU64 a_timeout = std::numeric_limits<mU64>::max());
    void push_waitOnSemaphores(
        std::vector<VkSemaphore> a_semaphoresToWait);

    mU64 signal_fence();
    void signal_semaphores(
        std::vector<VkSemaphore> a_semaphoresToSignal);

    void flush();

    VkCommandBuffer get_commandBuffer();
    void            submit_commandBuffer(VkCommandBuffer a_commandBuffer);

    [[NODISCARD]] VkQueue get_queue() { return m_queue; }
    mU32                  get_queueFamilyIndex() { return m_queueFamilyIndex; }

   protected:
    VkCommandPool   create_commandPool();
    VkCommandBuffer create_commandBuffer(VkCommandPool a_pool);

   private:
    mU32    m_queueFamilyIndex = 0;
    VkQueue m_queue            = VK_NULL_HANDLE;

    VkSemaphore m_timelineSemaphore;
    mU64        m_timeline = 0;

    // Keep track of command pools that are "in-flight"
    using CommandBufferQueue = std::queue<VkCommandBuffer>;

    struct CommandPoolEntry
    {
        mU64               fenceValue;
        VkCommandPool      commandPool;
        CommandBufferQueue availableCommandBuffers;
        CommandBufferQueue usedCommandBuffers;
    };

    using CommandPoolQueue = std::queue<CommandPoolEntry>;
    CommandPoolQueue m_inFlightCommandPools;
    CommandPoolQueue m_freeCommandPools;
};

class VulkanContext
{
   public:
    VulkanContext()                      = default;
    VulkanContext(VulkanContext const&)  = delete;
    VulkanContext(VulkanContext const&&) = delete;
    ~VulkanContext()                     = default;
    static VulkanContext* gs_VulkanContexte;
    void                  init();
    void                  deinit();

    static CommandQueue& get_commandQueue()
    {
        return gs_VulkanContexte->m_queue;
    }

    VkCommandBuffer get_singleUseCommandBuffer();
    void submit_singleUseCommandBuffer(VkCommandBuffer a_commandBuffer);

    static VkInstance get_instance() { return gs_VulkanContexte->m_instance; }
    static VkPhysicalDevice get_physDevice()
    {
        return gs_VulkanContexte->m_physicalDevice;
    }
    static VkDevice get_logDevice()
    {
        return gs_VulkanContexte->m_logicalDevice;
    }

    static mU32           get_memoryTypeIndex(mU32                  a_typeFilter,
                                              VkMemoryPropertyFlags a_properties);
    static VkShaderModule create_shaderModule(std::string const& a_shaderPath);

   private:
    // Instance and devices
    VkInstance       m_instance       = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice         m_logicalDevice  = VK_NULL_HANDLE;

    VkDebugUtilsMessengerEXT m_debugUtil;

    // queues
    CommandQueue m_queue;

    // Utility
    VkCommandPool m_utilityCommandPool;
};

};  // namespace vulkan
};  // namespace m

#endif  // M_VulkanContext