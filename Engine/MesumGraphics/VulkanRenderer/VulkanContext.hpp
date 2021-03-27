#ifndef M_VulkanContext
#define M_VulkanContext

#include <VulkanRendererCommon.hpp>

namespace m
{
namespace vulkan
{
void ImGui_RendererNewFrame();

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

   private:
    VkInstance m_instance;
    VkPhysicalDevice         m_physicalDevice;
    VkDebugUtilsMessengerEXT m_debugUtil;
};

void openRenderModule();
void closeRenderModule();
};  // namespace vulkan
};  // namespace m

#endif  // M_VulkanContext