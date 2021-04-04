#ifndef M_VulkanContext
#define M_VulkanContext

#include <VulkanRendererCommon.hpp>

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


    VkCommandBuffer get_singleUseCommandBuffer();
    void submit_signleUseCommandBuffer(VkCommandBuffer a_commandBuffer);

    VkInstance       get_instance() { return m_instance; }
    VkPhysicalDevice get_physDevice() { return m_physicalDevice; }
    VkDevice         get_logDevice() { return m_logicalDevice; }
    VkQueue          get_presentationQueue() { return m_queue; }
    VkQueue          get_graphicQueue() { return m_queue; }

   private:
    VkInstance       m_instance       = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice         m_logicalDevice  = VK_NULL_HANDLE;

    VkQueue m_queue = VK_NULL_HANDLE;

    VkCommandPool m_utilityCommandPool;

    VkDebugUtilsMessengerEXT m_debugUtil;
};

};  // namespace vulkan
};  // namespace m

#endif  // M_VulkanContext