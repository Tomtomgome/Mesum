#ifndef M_RenderTaskDearImGui
#define M_RenderTaskDearImGui
#pragma once

#include <MesumGraphics/RenderTask.hpp>
#include <MesumGraphics/Renderer.hpp>

#ifdef M_DX12_RENDERER
#include <MesumGraphics/DX12Renderer/DX12Context.hpp>
#endif // M_DX12_RENDERER
#ifdef M_VULKAN_RENDERER
#include <MesumGraphics/VulkanRenderer/VulkanContext.hpp>
#endif // M_VULKAN_RENDERER

namespace m::render
{
struct TaskDataDrawDearImGui : public TaskData
{
    ISurface::HdlPtr m_hdlOutput;

    mIfDx12Enabled(Task* getNew_dx12Implementation(TaskData* a_data) override);
    mIfVulkanEnabled(Task* getNew_vulkanImplementation(TaskData* a_data)
                         override);
};

struct TaskDrawDearImGui : public Task
{
    explicit TaskDrawDearImGui(TaskDataDrawDearImGui* a_data);

    TaskDataDrawDearImGui m_taskData;
};

mIfDx12Enabled(struct Dx12TaskDrawDearImGui : public TaskDrawDearImGui
{
    explicit Dx12TaskDrawDearImGui(TaskDataDrawDearImGui* a_data);
    ~Dx12TaskDrawDearImGui() override;

    void execute() const override;

   private:
    dx12::ComPtr<ID3D12DescriptorHeap> m_SRVDescriptorHeap;
};)

mIfVulkanEnabled(struct VulkanTaskDrawDearImGui : public TaskDrawDearImGui
{
    explicit VulkanTaskDrawDearImGui(TaskDataDrawDearImGui* a_data);
    ~VulkanTaskDrawDearImGui() override;

    void execute() const override;

   private:
    VkDescriptorPool m_dearImGuiDescriptorPool = VK_NULL_HANDLE;
};)

}  // namespace m::render

#endif  // M_RenderTaskDearImGui