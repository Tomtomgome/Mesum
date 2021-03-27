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


    VkInstance get_instance() { return m_instance; }
   private:
    VkInstance               m_instance       = VK_NULL_HANDLE;
    VkPhysicalDevice         m_physicalDevice = VK_NULL_HANDLE;
    VkDevice                 m_logicalDevice  = VK_NULL_HANDLE;

    VkQueue                  m_queue = VK_NULL_HANDLE;

    VkDebugUtilsMessengerEXT m_debugUtil;
};

};  // namespace vulkan
};  // namespace m

#endif  // M_VulkanContext