#pragma once

#include "RenderTask.hpp"
#include <MesumGraphics/Renderer.hpp>
#include <MesumGraphics/MesumGraphics/Common.hpp>

#ifdef M_DX12_RENDERER
#include <MesumGraphics/DX12Renderer/DX12Context.hpp>
#endif  // M_DX12_RENDERER
#ifdef M_VULKAN_RENDERER
#include <MesumGraphics/VulkanRenderer/VulkanContext.hpp>
#endif  // M_VULKAN_RENDERER

namespace m::render
{
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct mTaskDataSwapchainWaitForRT : public TaskData
{
    mISwapchain* pSwapchain = nullptr;
    mISynchTool* pSynchTool = nullptr;

    mIfDx12Enabled(Task* getNew_dx12Implementation(TaskData* a_data) override);
    mIfVulkanEnabled(Task* getNew_vulkanImplementation(TaskData* a_data)
                         override);
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct mTaskSwapchainWaitForRT : public Task
{
    explicit mTaskSwapchainWaitForRT(mTaskDataSwapchainWaitForRT* a_data);

    void prepare() override {}

    mTaskDataSwapchainWaitForRT taskData;
    mIRenderTarget*             pOutputRT = nullptr;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
mIfDx12Enabled(struct mTaskSwapchainWaitForRTDx12 : public mTaskSwapchainWaitForRT
{
    explicit mTaskSwapchainWaitForRTDx12(mTaskDataSwapchainWaitForRT* a_data);
    ~mTaskSwapchainWaitForRTDx12() override;

    void execute() const override;
};)

//-----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
mIfVulkanEnabled(struct mTaskSwapchainWaitForRTVulkan : public mTaskSwapchainWaitForRT
{
    explicit mTaskSwapchainWaitForRTVulkan(mTaskDataSwapchainWaitForRT* a_data);

    void execute() const override;
};)



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct mTaskDataSwapchainPresent : public TaskData
{
    mISwapchain* pSwapchain = nullptr;
    mISynchTool* pSynchTool = nullptr;

    mIfDx12Enabled(Task* getNew_dx12Implementation(TaskData* a_data) override);
    mIfVulkanEnabled(Task* getNew_vulkanImplementation(TaskData* a_data)
                         override);
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct mTaskSwapchainPresent : public Task
{
    explicit mTaskSwapchainPresent(mTaskDataSwapchainPresent* a_data);

    void prepare() override {}

    mTaskDataSwapchainPresent taskData;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
mIfDx12Enabled(struct mTaskSwapchainPresentDx12 : public mTaskSwapchainPresent
{
    explicit mTaskSwapchainPresentDx12(mTaskDataSwapchainPresent* a_data);

    void execute() const override;
};)

//-----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
mIfVulkanEnabled(struct mTaskSwapchainPresentVulkan : public mTaskSwapchainPresent
{
    explicit mTaskSwapchainPresentVulkan(mTaskDataSwapchainPresent* a_data);

    void execute() const override;
};)

}  // namespace m::render