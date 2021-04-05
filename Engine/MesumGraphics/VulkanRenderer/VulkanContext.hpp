#ifndef M_VulkanContext
#define M_VulkanContext

#include <VulkanRendererCommon.hpp>
#include <vector>

namespace m
{
namespace vulkan
{
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

    static void wait_onMainTimelineTstp(
        U64 a_tstpToWaitOn, U64 a_timeout = std::numeric_limits<U64>::max());
    static U64 submit_onMainTimeline(
        std::vector<VkSemaphore> const& a_semaphoresToWait,
        std::vector<VkSemaphore>        a_semaphoresToSignal);

    VkCommandBuffer get_singleUseCommandBuffer();
    void submit_signleUseCommandBuffer(VkCommandBuffer a_commandBuffer);

    VkInstance       get_instance() { return m_instance; }
    VkPhysicalDevice get_physDevice() { return m_physicalDevice; }
    VkDevice         get_logDevice() { return m_logicalDevice; }
    VkQueue          get_presentationQueue() { return m_queue; }
    VkQueue          get_graphicQueue() { return m_queue; }

   private:
    // Instance and devices
    VkInstance       m_instance       = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice         m_logicalDevice  = VK_NULL_HANDLE;

    VkDebugUtilsMessengerEXT m_debugUtil;

    // queues
    VkQueue m_queue = VK_NULL_HANDLE;

    // Utility
    VkCommandPool m_utilityCommandPool;

    // Synchronization tools
    VkSemaphore m_timelineSemaphore;
    U64         m_timeline = 0;
};

};  // namespace vulkan
};  // namespace m

#endif  // M_VulkanContext