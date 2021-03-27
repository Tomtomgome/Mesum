#include <VulkanContext.hpp>

namespace m
{
namespace vulkan
{
void ImGui_RendererNewFrame() {}

VulkanContext* VulkanContext::gs_VulkanContexte;

void VulkanContext::init() {
    create_instance(m_instance);

    setup_debugMessenger(m_instance, m_debugUtil);
}

void VulkanContext::deinit() {
    vkDestroyInstance(m_instance, NULL);
}

void openRenderModule()
{
    VulkanContext::gs_VulkanContexte = new VulkanContext();
    VulkanContext::gs_VulkanContexte->init();
}
void closeRenderModule()
{
    VulkanContext::gs_VulkanContexte->deinit();
    delete VulkanContext::gs_VulkanContexte;
}
}  // namespace vulkan
}  // namespace m