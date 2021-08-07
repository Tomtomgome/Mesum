#ifndef M_RenderTaskDearImGui
#define M_RenderTaskDearImGui
#pragma once

#include <MesumGraphics/DX12Renderer/DX12Context.hpp>
#include <MesumGraphics/RenderTask.hpp>
#include <MesumGraphics/Renderer.hpp>
#include <MesumGraphics/VulkanRenderer/VulkanContext.hpp>

namespace m::render
{
struct TaskDataDrawDearImGui : public TaskData
{
    ISurface::HdlPtr m_hdlOutput;

    Task* getNew_dx12Implementation(TaskData* a_data) override;
    Task* getNew_vulkanImplementation(TaskData* a_data) override;
};

struct TaskDrawDearImGui : public Task
{
    explicit TaskDrawDearImGui(TaskDataDrawDearImGui* a_data);

    TaskDataDrawDearImGui m_taskData;
};

struct Dx12TaskDrawDearImGui : public TaskDrawDearImGui
{
    explicit Dx12TaskDrawDearImGui(TaskDataDrawDearImGui* a_data);
    ~Dx12TaskDrawDearImGui() override;

    void execute() const override;

   private:
    dx12::ComPtr<ID3D12DescriptorHeap> m_SRVDescriptorHeap;
};

struct VulkanTaskDrawDearImGui : public TaskDrawDearImGui
{
    explicit VulkanTaskDrawDearImGui(TaskDataDrawDearImGui* a_data);
    ~VulkanTaskDrawDearImGui() override;

    void execute() const override;

   private:
    VkDescriptorPool m_dearImGuiDescriptorPool = VK_NULL_HANDLE;
};

}  // namespace m::render

#endif  // M_RenderTaskDearImGui