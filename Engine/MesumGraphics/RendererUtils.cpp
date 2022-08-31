#include "RendererUtils.hpp"

#include "Renderer.hpp"
#include "Windows.hpp"

#include "RenderTasks/RenderTasksBasicSwapchain.hpp"

#if defined M_WIN32
#include "Win32/WindowsImpl.hpp"
#elif defined M_UNIX
// TODO : linux support :(
#endif

#include <algorithm>

namespace m::render
{
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mTasksetExecutor::init() {}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mTasksetExecutor::destroy()
{
    for (auto [api, taskset] : oneTimeTasksets)
    {
        unref_safe(taskset).clear();
        api->destroy_renderTaskset(unref_safe(taskset));
    }
    oneTimeTasksets.clear();
    std::vector<std::pair<mIApi const*, Taskset*>>().swap(oneTimeTasksets);

    for (auto [api, taskset] : permanentTasksets)
    {
        unref_safe(taskset).clear();
        api->destroy_renderTaskset(unref_safe(taskset));
    }
    permanentTasksets.clear();
    std::list<std::pair<mIApi const*, Taskset*>>().swap(permanentTasksets);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mTasksetExecutor::run()
{
    for (auto [api, taskset] : oneTimeTasksets)
    {
        taskset->execute();
        taskset->clear();
        api->destroy_renderTaskset(unref_safe(taskset));
    }
    oneTimeTasksets.clear();

    for (auto [api, taskset] : permanentTasksets) { taskset->execute(); }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mTasksetExecutor::confy_oneTimeTaskset(mIApi const& a_api,
                                            Taskset&     a_taskset)
{
    oneTimeTasksets.push_back(std::make_pair(&a_api, &a_taskset));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mTasksetExecutor::confy_permanentTaskset(mIApi const& a_api,
                                              Taskset&     a_taskset)
{
    permanentTasksets.push_back(std::make_pair(&a_api, &a_taskset));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mTasksetExecutor::remove_permanentTaskset(mIApi const& a_api,
                                               Taskset&     a_taskset)
{
    auto itTasksetToRemove =
        std::find(permanentTasksets.begin(), permanentTasksets.end(),
                  std::make_pair(&a_api, &a_taskset));
    mAssert(itTasksetToRemove != permanentTasksets.end());

    unref_safe(itTasksetToRemove->second).clear();
    (itTasksetToRemove->first)
        ->destroy_renderTaskset(unref_safe(itTasksetToRemove->second));
    permanentTasksets.erase(itTasksetToRemove);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void init_swapchainWithWindow(mIApi const& a_api, mTasksetExecutor& a_executor,
                              mISwapchain&       a_swapchain,
                              mISynchTool&       a_synchTool,
                              windows::mIWindow& a_window,
                              mUInt const        a_nbBackbuffer)
{
    m::render::mISwapchain::Desc scDesc{};
    scDesc.bufferCount   = a_nbBackbuffer;
    auto [width, height] = a_window.get_size();
    scDesc.width         = width;
    scDesc.height        = height;

#if defined M_WIN32
    m::render::mISwapchain::DescWin32 scDescWin32{};
    scDescWin32.hwd = static_cast<m::win32::IWindowImpl&>(a_window).get_hwnd();
    a_swapchain.init_win32(scDesc, scDescWin32);
#elif defined M_UNIX
    // TODO : linux support :(
#endif

    auto produceResizeTaskSet = [&](mU32 a_width, mU32 a_height)
    {
        auto& taskSet = a_api.create_renderTaskset();

        mTaskDataSwapchainWaitForRT flushData{};
        flushData.pSwapchain = &a_swapchain;
        flushData.pSynchTool = &a_synchTool;
        flushData.flush      = true;
        flushData.add_toTaskSet(taskSet);

        mTaskDataSwapchainResize resizeData{};
        resizeData.pSwapchain = &a_swapchain;
        resizeData.width      = a_width;
        resizeData.height     = a_height;
        resizeData.add_toTaskSet(taskSet);

        a_executor.confy_oneTimeTaskset(a_api, taskSet);
    };

    a_window.attach_toResize(
        windows::mIWindow::mOnResizeCallback(produceResizeTaskSet));

    a_window.attach_toDestroy(
        mCallback<void>([&a_swapchain]() { a_swapchain.destroy(); }));
}
}  // namespace m::render