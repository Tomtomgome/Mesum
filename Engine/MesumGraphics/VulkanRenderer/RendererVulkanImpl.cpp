#include <RendererVulkanImpl.hpp>

namespace m
{
namespace vulkan
{
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
void VulkanRenderer::init()
{
    //     DX12Context::gs_dx12Contexte = new DX12Context();
    //     DX12Context::gs_dx12Contexte->init();
}

void VulkanRenderer::destroy()
{
    //     DX12Context::gs_dx12Contexte->deinit();
    //     delete DX12Context::gs_dx12Contexte;
    // #ifdef M_DEBUG
    //     dx12::report_liveObjects();
    // #endif  // M_DEBUG
}

void VulkanRenderer::start_dearImGuiNewFrame()
{
    //     ImGui_ImplDX12_NewFrame();
    //     DX12Context::gs_dx12Contexte->m_dearImGuiPlatImplCallback.call();
}

render::ISurface* VulkanRenderer::get_newSurface()
{
    return nullptr;
    /*    return new DX12Surface();*/
}
}  // namespace vulkan
}  // namespace m