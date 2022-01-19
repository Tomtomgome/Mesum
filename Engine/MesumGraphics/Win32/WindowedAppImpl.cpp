#include <DearImgui/imgui.h>

#include <CrossRenderer.hpp>
#include <WindowedAppImpl.hpp>
#include <WindowsImpl.hpp>

#include "imgui_impl_win32.h"

namespace m::win32
{
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND a_hwnd, UINT a_uMsg, WPARAM a_wParam,
                            LPARAM a_lParam)
{
    IWindowImpl* dataApp =
        reinterpret_cast<IWindowImpl*>(GetWindowLongPtr(a_hwnd, GWLP_USERDATA));
    if (dataApp != NULL)
    {
        return dataApp->process_messages(a_uMsg, a_wParam, a_lParam);
    }
    else
    {
        return DefWindowProcW(a_hwnd, a_uMsg, a_wParam, a_lParam);
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
windows::mIWindow* IWindowedApplicationImpl::add_newWindow(std::string a_name,
                                                           mU32        a_width,
                                                           mU32        a_height)
{
    IWindowImpl* newWindow = new IWindowImpl();
    m_windows.insert(newWindow);

    newWindow->set_winContext(m_W32Context);
    newWindow->init({a_name, {a_width, a_height}});

    return newWindow;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void IWindowedApplicationImpl::start_dearImGuiNewFrame(
    render::IRenderer const* a_renderer) const
{
    a_renderer->start_dearImGuiNewFrameRenderer();
    ImGui_ImplWin32_NewFrame();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void IWindowedApplicationImpl::init(mCmdLine const& a_cmdLine, void* a_appData)
{
    WindowedLaunchData& data = *(WindowedLaunchData*)a_appData;

    m_W32Context.init(data.m_hInstance);

    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Using this awareness context allows the client area of the window
    // to achieve 100% scaling while still allowing non-client window content to
    // be rendered in a DPI sensitive fashion.
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    const mWideChar className[] = L"MainWindowClass";
    // Register the window class.
    m_W32Context.register_windowClass(className, data.m_hInstance, WindowProc);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void IWindowedApplicationImpl::destroy()
{
    for (auto element = m_windows.begin(); element != m_windows.end();
         ++element)
    {
        windows::mIWindow* window = (*element);
        window->destroy();
        delete window;
    }
    m_windows.clear();

    m_W32Context.destroy();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void IWindowedApplicationImpl::process_messages()
{
    // Run the message loop.
    MSG msg = {};
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            m_signalKeepRunning = false;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
mBool IWindowedApplicationImpl::step(
    std::chrono::steady_clock::duration const& a_deltaTime)
{
    process_messages();

    auto element = m_windows.begin();
    while (element != m_windows.end())
    {
        windows::mIWindow* window = (*element);
        if (static_cast<IWindowImpl*>(window)->is_flaggedToBeClosed())
        {
            window->destroy();
            delete window;

            auto oldElem = element;
            element++;
            m_windows.erase(oldElem);
        }
        else
        {
            element++;
        }
    }

    if (m_windows.size() == 0)
    {
        m_signalKeepRunning = false;
    }

    return m_signalKeepRunning;
}
};  // namespace m::win32