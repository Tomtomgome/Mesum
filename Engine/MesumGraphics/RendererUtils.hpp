#pragma once

namespace m::windows
{
class mIWindow;
}

namespace m::render
{
class mISwapchain;
class mIApi;

void init_swapchainWithWindow(mIApi const& a_api, mISwapchain& a_swapchain,
                              windows::mIWindow& a_window);
};  // namespace m::render