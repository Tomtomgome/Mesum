#ifndef M_VulkanContext
#define M_VulkanContext

#include <MesumCore/Kernel/Callbacks.hpp>
#include <VulkanRendererCommon.hpp>

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
    mU64 signal_fence(std::vector<VkSemaphore> const& a_semaphoresToWait,
                      std::vector<VkSemaphore>        a_semaphoresToSignal);

    //    mU64 submit_onMainTimeline(
    //        VkCommandBuffer const&          a_commandBuffer,
    //        std::vector<VkSemaphore> const& a_semaphoresToWait,
    //        std::vector<VkSemaphore>        a_semaphoresToSignal);

    [[NODISCARD]] VkQueue get_queue() { return m_queue; }

   private:
    mU32    m_queueFamilyIndex = 0;
    VkQueue m_queue            = VK_NULL_HANDLE;

    VkSemaphore m_timelineSemaphore;
    mU64        m_timeline = 0;
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

    //    static void wait_onMainTimelineTstp(
    //        mU64 a_tstpToWaitOn, mU64 a_timeout =
    //        std::numeric_limits<mU64>::max());
    //    static mU64 submit_onMainTimeline(
    //        VkCommandBuffer const&          a_commandBuffer,
    //        std::vector<VkSemaphore> const& a_semaphoresToWait,
    //        std::vector<VkSemaphore>        a_semaphoresToSignal);

    // static void present(VkPresentInfoKHR const& a_infoPresent);

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
    //    static VkQueue get_presentationQueue()
    //    {
    //        return gs_VulkanContexte->m_queue;
    //    }
    //    static VkQueue get_graphicQueue() { return gs_VulkanContexte->m_queue;
    //    } static mU32    get_graphicQueueFamilyIndex()
    //    {
    //        return gs_VulkanContexte->m_queueFamilyIndex;
    //    }

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

    // Synchronization tools
    VkSemaphore m_timelineSemaphore;
    mU64        m_timeline = 0;
};

};  // namespace vulkan
};  // namespace m

#endif  // M_VulkanContext