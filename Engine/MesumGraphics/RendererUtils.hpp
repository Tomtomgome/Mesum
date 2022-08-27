#pragma once

namespace m::windows
{
class mIWindow;
}

namespace m::render
{
class mISwapchain;
class mIApi;
struct Taskset;

struct mTasksetExecutor
{
    void init();
    void destroy();

    void run();
    void confy_oneTimeTaskset(mIApi const& a_api, Taskset& a_taskset);
    void confy_permanentTaskset(mIApi const& a_api, Taskset& a_taskset);

    std::vector<std::pair<mIApi const*, Taskset*>> oneTimeTasksets;
    std::vector<std::pair<mIApi const*, Taskset*>> permanentTasksets;
};

void init_swapchainWithWindow(mIApi const& a_api, mTasksetExecutor& a_executor,
                              mISwapchain&       a_swapchain,
                              mISynchTool&       a_synchTool,
                              windows::mIWindow& a_window,
                              mUInt const        a_nbBackbuffer);
};  // namespace m::render