#include <RenderTaskDearImGui.hpp>

#ifdef M_DX12_RENDERER
#include <imgui_impl_dx12.h>

#include <MesumGraphics/DX12Renderer/RendererDX12Impl.hpp>
#endif  // M_DX12_RENDERER

#ifdef M_VULKAN_RENDERER
#include <imgui_impl_vulkan.h>

#include <MesumGraphics/VulkanRenderer/RendererVulkanImpl.hpp>
#endif  // M_VULKAN_RENDERER

namespace m::render
{
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
TaskDrawDearImGui::TaskDrawDearImGui(TaskDataDrawDearImGui* a_data)
{
    mSoftAssert(a_data != nullptr);
    m_taskData = *a_data;
}

#ifdef M_DX12_RENDERER
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Dx12TaskDrawDearImGui::Dx12TaskDrawDearImGui(TaskDataDrawDearImGui* a_data)
    : TaskDrawDearImGui(a_data)
{
    mAssert(a_data->nbFrames != 0);
    mAssert(a_data->pOutputRT != nullptr);
    m_SRVDescriptorHeap = dx12::create_descriptorHeap(
        dx12::DX12Context::gs_dx12Contexte->m_device,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, a_data->nbFrames,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

    ImGui_ImplDX12_Init(
        dx12::DX12Context::gs_dx12Contexte->m_device.Get(), a_data->nbFrames,
        DXGI_FORMAT_B8G8R8A8_UNORM, m_SRVDescriptorHeap.Get(),
        m_SRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        m_SRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Dx12TaskDrawDearImGui::~Dx12TaskDrawDearImGui()
{
    ImGui_ImplDX12_Shutdown();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskDrawDearImGui::execute() const
{
    dx12::ComPtr<ID3D12GraphicsCommandList2> graphicCommandList =
        dx12::DX12Context::gs_dx12Contexte->get_commandQueue()
            .get_commandList();

    auto pOutputRT =
        static_cast<dx12::mRenderTarget const*>(m_taskData.pOutputRT);

    D3D12_CPU_DESCRIPTOR_HANDLE const rtv = pOutputRT->rtv;
    graphicCommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

    graphicCommandList->SetDescriptorHeaps(1,
                                           m_SRVDescriptorHeap.GetAddressOf());
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(),
                                  graphicCommandList.Get());

    dx12::DX12Context::gs_dx12Contexte->get_commandQueue().execute_commandList(
        graphicCommandList.Get());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Task* TaskDataDrawDearImGui::getNew_dx12Implementation(TaskData* a_data)
{
    return new Dx12TaskDrawDearImGui(
        dynamic_cast<TaskDataDrawDearImGui*>(a_data));
}
#endif  // M_DX12_RENDERER

#ifdef M_VULKAN_RENDERER
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
VulkanTaskDrawDearImGui::VulkanTaskDrawDearImGui(TaskDataDrawDearImGui* a_data)
    : TaskDrawDearImGui(a_data)
{
    // TODO modify this to work with swapchain refacto
    //    auto currentSurface =
    //        static_cast<vulkan::VulkanSurface*>(m_taskData.m_hdlOutput->surface);
    //
    //    VkDescriptorPoolSize pool_sizes[] = {
    //        {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
    //        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
    //        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
    //        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
    //        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
    //        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
    //        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
    //        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
    //        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
    //        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
    //        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
    //    VkDescriptorPoolCreateInfo pool_info = {};
    //    pool_info.sType         =
    //    VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO; pool_info.flags =
    //    VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; pool_info.maxSets
    //    = 1000 * IM_ARRAYSIZE(pool_sizes); pool_info.poolSizeCount =
    //    (uint32_t)IM_ARRAYSIZE(pool_sizes); pool_info.pPoolSizes    =
    //    pool_sizes;
    //    vkCreateDescriptorPool(vulkan::VulkanContext::get_logDevice(),
    //    &pool_info,
    //                           nullptr, &m_dearImGuiDescriptorPool);
    //
    //    ImGui_ImplVulkan_InitInfo init_info = {};
    //    init_info.Instance                  =
    //    vulkan::VulkanContext::get_instance(); init_info.PhysicalDevice =
    //    vulkan::VulkanContext::get_physDevice(); init_info.Device         =
    //    vulkan::VulkanContext::get_logDevice(); init_info.QueueFamily =
    //        vulkan::VulkanContext::get_graphicQueueFamilyIndex();
    //    init_info.Queue           = vulkan::VulkanContext::get_graphicQueue();
    //    init_info.PipelineCache   = VK_NULL_HANDLE;
    //    init_info.DescriptorPool  = m_dearImGuiDescriptorPool;
    //    init_info.Allocator       = nullptr;
    //    init_info.MinImageCount   = vulkan::VulkanSurface::scm_numFrames;
    //    init_info.ImageCount      = vulkan::VulkanSurface::scm_numFrames;
    //    init_info.CheckVkResultFn = vulkan::check_vkResult;
    //    ImGui_ImplVulkan_Init(&init_info,
    //    currentSurface->get_mainRenderPass());
    //
    //    VkCommandBuffer command_buffer =
    //        vulkan::VulkanContext::gs_VulkanContexte->get_singleUseCommandBuffer();
    //
    //    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
    //
    //    vulkan::VulkanContext::gs_VulkanContexte->submit_singleUseCommandBuffer(
    //        command_buffer);
    //
    //    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
VulkanTaskDrawDearImGui::~VulkanTaskDrawDearImGui()
{
    ImGui_ImplVulkan_Shutdown();

    vkDestroyDescriptorPool(vulkan::VulkanContext::get_logDevice(),
                            m_dearImGuiDescriptorPool, nullptr);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanTaskDrawDearImGui::execute() const
{
    // TODO modify this to work with swapchain refacto
    //    auto currentSurface =
    //        static_cast<vulkan::VulkanSurface*>(m_taskData.m_hdlOutput->surface);
    //    auto framebuffer   = currentSurface->get_currentFramebuffer();
    //    auto commandBuffer = currentSurface->get_currentCommandBuffer();
    //
    //    {
    //        VkRenderPassBeginInfo info   = {};
    //        info.sType                   =
    //        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO; info.renderPass =
    //        currentSurface->get_mainRenderPass(); info.framebuffer =
    //        framebuffer; info.renderArea.extent.width =
    //        currentSurface->get_width(); info.renderArea.extent.height =
    //        currentSurface->get_height(); VkClearValue clearValues[1]   = {};
    //        clearValues[0].color          = {0.4f, 0.6f, 0.9f, 1.0f};
    //        info.clearValueCount          = 1;
    //        info.pClearValues             = clearValues;
    //        vkCmdBeginRenderPass(commandBuffer, &info,
    //        VK_SUBPASS_CONTENTS_INLINE);
    //    }
    //
    //    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    //
    //    // Submit command buffer
    //    vkCmdEndRenderPass(commandBuffer);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Task* TaskDataDrawDearImGui::getNew_vulkanImplementation(TaskData* a_data)
{
    return new VulkanTaskDrawDearImGui(
        static_cast<TaskDataDrawDearImGui*>(a_data));
}
#endif  // M_VULKAN_RENDERER
}  // namespace m::render