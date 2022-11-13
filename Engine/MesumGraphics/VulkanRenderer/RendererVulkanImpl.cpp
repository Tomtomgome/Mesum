#include <imgui_impl_vulkan.h>

#include <RendererVulkanImpl.hpp>
#include <VulkanContext.hpp>
#include <limits>

namespace m::vulkan
{
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
render::Task* VulkanRenderTaskset::add_task(render::TaskData* a_data)
{
    auto task = a_data->getNew_vulkanImplementation(a_data);
    m_set_tasks.push_back(task);
    return task;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void VulkanRenderTaskset::clear()
{
    for (auto task : m_set_tasks) { delete task; }
    m_set_tasks.clear();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void VulkanRenderTaskset::execute()
{
    for (const auto task : m_set_tasks) { task->prepare(); }
    for (const auto task : m_set_tasks) { task->execute(); }
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSynchTool::init(Desc& a_desc)
{
    fenceValues.resize(a_desc.bufferCount, 0);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSynchTool::destroy()
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
memory::mMemoryType      mApi::sm_memoryType = memory::g_defaultMemoryType;
memory::mObjectAllocator mApi::sm_mal;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mApi::init()
{
    // Two vulkan Api shouldn't be initialized
    mAssert(mApi::sm_memoryType == memory::g_defaultMemoryType);
    mApi::sm_memoryType = memory::create_newMemoryType("Vulkan rendering");
    sm_mal.init(mApi::sm_memoryType);

    VulkanContext::gs_VulkanContexte = sm_mal.construct<VulkanContext>();
    VulkanContext::gs_VulkanContexte->init();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mApi::destroy()
{
    VulkanContext::gs_VulkanContexte->deinit();
    sm_mal.destroy(VulkanContext::gs_VulkanContexte);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mApi::start_dearImGuiNewFrameRenderer() const
{
    ImGui_ImplVulkan_NewFrame();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
render::mISwapchain& mApi::create_swapchain() const
{
    return sm_mal.construct_ref<mSwapchain>();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mApi::destroy_swapchain(render::mISwapchain& a_swapchain) const
{
    auto& vulkanSwapchain = dynamic_cast<mSwapchain&>(a_swapchain);
    sm_mal.destroy_ref(vulkanSwapchain);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
render::Taskset& mApi::create_renderTaskset() const
{
    return sm_mal.construct_ref<VulkanRenderTaskset>();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mApi::destroy_renderTaskset(render::Taskset& a_taskset) const
{
    auto& vulkanTaskset = dynamic_cast<VulkanRenderTaskset&>(a_taskset);
    sm_mal.destroy_ref(vulkanTaskset);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
render::mISynchTool& mApi::create_synchTool() const
{
    return sm_mal.construct_ref<mSynchTool>();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mApi::destroy_synchTool(render::mISynchTool& a_synchTool) const
{
    auto& vulkanSynchTool = dynamic_cast<mSynchTool&>(a_synchTool);
    sm_mal.destroy_ref(vulkanSynchTool);
}

}  // namespace m::vulkan