#include "RendererUtils.hpp"

#include "Renderer.hpp"
#include "Windows.hpp"

#if defined M_WIN32
#include "Win32/WindowsImpl.hpp"
#elif defined M_UNIX
// TODO : linux support :(
#endif

namespace m::render
{
void init_swapchainWithWindow(mIApi const& a_api, mISwapchain& a_swapchain,
                              windows::mIWindow& a_window)
{
    m::render::mISwapchain::Desc scDesc{};
    scDesc.bufferCount   = 3;
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

    a_window.attach_toResize(mCallback<void, mU32, mU32>(
        &a_swapchain, &render::mISwapchain::resize));

    a_window.attach_toDestroy(mCallback<void>(
        [&a_swapchain, &a_api]() { a_api.destroy_swapchain(a_swapchain); }));
}
}  // namespace m::render