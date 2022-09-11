#include <imgui_impl_dx12.h>

#include <DX12Context.hpp>
#include <RendererDX12Impl.hpp>

namespace m::dx12
{
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
render::Task* DX12RenderTaskset::add_task(render::TaskData* a_data)
{
    auto task = a_data->getNew_dx12Implementation(a_data);
    m_set_tasks.push_back(task);
    return task;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DX12RenderTaskset::clear()
{
    for (auto task : m_set_tasks) { delete task; }
    m_set_tasks.clear();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DX12RenderTaskset::execute()
{
    for (const auto task : m_set_tasks) { task->prepare(); }
    for (const auto task : m_set_tasks) { task->execute(); }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void mSynchToolDX12::init(Desc& a_desc)
{
    fenceValues.resize(a_desc.bufferCount);
}

void mSynchToolDX12::destroy()
{

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
memory::mMemoryType      mApiDX12::sm_memoryType = memory::g_defaultMemoryType;
memory::mObjectAllocator mApiDX12::sm_mal;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mApiDX12::init()
{
    // Two Dx12Api shouldn't be initialized
    mAssert(mApiDX12::sm_memoryType == memory::g_defaultMemoryType);
    mApiDX12::sm_memoryType = memory::create_newMemoryType("Dx12 rendering");
    sm_mal.init(mApiDX12::sm_memoryType);

    DX12Context::gs_dx12Contexte = sm_mal.construct<DX12Context>();
    DX12Context::gs_dx12Contexte->init();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mApiDX12::destroy()
{
    DX12Context::gs_dx12Contexte->deinit();
    sm_mal.destroy(DX12Context::gs_dx12Contexte);
#ifdef M_DEBUG
    dx12::report_liveObjects();
#endif  // M_DEBUG
    // check dx12 memory allocator
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mApiDX12::start_dearImGuiNewFrameRenderer() const
{
    ImGui_ImplDX12_NewFrame();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
render::mISwapchain& mApiDX12::create_swapchain() const
{
    return sm_mal.construct_ref<mSwapchainDX12>();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mApiDX12::destroy_swapchain(render::mISwapchain& a_swapchain) const
{
    auto& dx12Swapchain = dynamic_cast<mSwapchainDX12&>(a_swapchain);
    sm_mal.destroy_ref(dx12Swapchain);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
render::Taskset& mApiDX12::create_renderTaskset() const
{
    return sm_mal.construct_ref<DX12RenderTaskset>();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mApiDX12::destroy_renderTaskset(render::Taskset& a_taskset) const
{
    auto& dx12Taskset = dynamic_cast<DX12RenderTaskset&>(a_taskset);
    sm_mal.destroy_ref(dx12Taskset);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
render::mISynchTool& mApiDX12::create_synchTool() const
{
    return sm_mal.construct_ref<mSynchToolDX12>();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mApiDX12::destroy_synchTool(render::mISynchTool& a_synchTool) const
{
    auto& dx12SynchTool = dynamic_cast<mSynchToolDX12&>(a_synchTool);
    sm_mal.destroy_ref(dx12SynchTool);
}

}  // namespace m::dx12