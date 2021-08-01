#include <Win32Context.hpp>
#include <WindowsImpl.hpp>
#ifndef UNICODE
#define UNICODE
#endif

#include <imgui.h>
#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND   hWnd,
                                                             UINT   msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);
namespace m::win32
{
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
LRESULT IWindowImpl::process_messages(UINT a_uMsg, WPARAM a_wParam,
                                      LPARAM a_lParam)
{
    bool interrupt = false;
    m_signalOverrideInputProcessing.call(&interrupt, m_hwnd, a_uMsg, a_wParam,
                                         a_lParam);
    if (interrupt)
    {
        return true;
    }

    LRESULT result = 0;
    switch (a_uMsg)
    {
        case WM_KEYDOWN:
        {
            if (m_linkedInputManager != nullptr)
            {
                input::Key k = m_parentContext->get_keyFromParam(a_wParam);

                m_linkedInputManager->process_KeyEvent(
                    k, 0, input::Action::PRESSED, input::KeyMod::NONE);
            }
        }
        break;

        case WM_KEYUP:
        {
            if (m_linkedInputManager != nullptr)
            {
                input::Key k = m_parentContext->get_keyFromParam(a_wParam);

                m_linkedInputManager->process_KeyEvent(
                    k, 0, input::Action::RELEASED, input::KeyMod::NONE);
            }
        }
        break;

        case WM_DESTROY:
        {
            if (m_isMainWindow)
            {
                PostQuitMessage(0);
            }

            m_flagToBeClosed = true;
        }
        break;

        case WM_SIZE:
        {
            RECT clientRect = {};
            ::GetClientRect(m_hwnd, &clientRect);

            U32 width  = clientRect.right - clientRect.left;
            U32 height = clientRect.bottom - clientRect.top;

            m_resizeSignal.call(width, height);
        }
        break;
        default: result = DefWindowProcW(m_hwnd, a_uMsg, a_wParam, a_lParam);
    }
    return result;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void IWindowImpl::init()
{
    const WideChar className[] = L"MainWindowClass";
    m_hwnd = m_parentContext->create_window(className, m_windowName,
                                            m_clientWidth, m_clientHeight);
    GetWindowRect(m_hwnd, &m_windowRect);

    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, LONG_PTR(this));

    ShowWindow(m_hwnd, SW_NORMAL);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void IWindowImpl::destroy()
{
    m_signalWindowDestroyed.call();

    ::DestroyWindow(m_hwnd);
    m_hwnd = NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
render::ISurface::HdlPtr IWindowImpl::link_renderer(
    render::IRenderer* a_renderer)
{
    mHardAssert(a_renderer != nullptr);
    render::ISurface::HdlPtr surfaceHandle =
        std::make_shared<render::ISurface::Handle>();
    surfaceHandle->surface                   = a_renderer->get_newSurface();
    render::Win32SurfaceInitData surfaceData = {m_hwnd, m_clientWidth,
                                                m_clientHeight};
    surfaceHandle->surface->init_win32(surfaceData);

    m_resizeSignal.attach_ToSignal(Callback<void, U32, U32>(
        surfaceHandle->surface, &render::ISurface::resize));

    // TODO : Manage this from the renderer
    m_signalWindowDestroyed.attach_ToSignal(Callback<void>(
        [surfaceHandle]()
        {
            surfaceHandle->isValid = false;
            surfaceHandle->surface->destroy();
            delete surfaceHandle->surface;
        }));

    return surfaceHandle;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void IWindowImpl::set_asMainWindow()
{
    static m::Bool s_mainWindowIsDefined = false;

    // There can only be one main window
    mHardAssert(s_mainWindowIsDefined == false);
    mHardAssert(m_isMainWindow == false);
    s_mainWindowIsDefined = true;
    m_isMainWindow        = true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void IWindowImpl::set_asImGuiWindow()
{
    // There can only be one ImGui window, and it's the main one
    mHardAssert(m_isMainWindow == true);

    ImGui_ImplWin32_Init(m_hwnd);

    m_signalWindowDestroyed.attach_ToSignal(
        Callback<void>([]() { ImGui_ImplWin32_Shutdown(); }));

    m_signalOverrideInputProcessing.attach_ToSignal(CallbackInputProcessing(
        [](Bool* a_interrupt, HWND a_hwnd, UINT a_uMsg, WPARAM a_wParam,
           LPARAM a_lParam)
        {
            if (!(*a_interrupt))
            {
                *a_interrupt = ImGui_ImplWin32_WndProcHandler(
                    a_hwnd, a_uMsg, a_wParam, a_lParam);
            }
        }));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void IWindowImpl::set_fullScreen(Bool a_fullscreen)
{
    if (m_fullscreen != a_fullscreen)
    {
        toggle_fullScreen();
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void IWindowImpl::toggle_fullScreen()
{
    m_fullscreen = !m_fullscreen;

    if (m_fullscreen)
    {
        GetWindowRect(m_hwnd, &m_windowRect);

        UINT windowStyle =
            WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME |
                                    WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

        SetWindowLongW(m_hwnd, GWL_STYLE, windowStyle);
        // Query the name of the nearest display device for the window.
        // This is required to set the fullscreen dimensions of the window
        // when using a multi-monitor setup.
        HMONITOR hMonitor =
            ::MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFOEX monitorInfo = {};
        monitorInfo.cbSize        = sizeof(MONITORINFOEX);
        GetMonitorInfo(hMonitor, &monitorInfo);
        SetWindowPos(m_hwnd, HWND_TOP, monitorInfo.rcMonitor.left,
                     monitorInfo.rcMonitor.top,
                     monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                     monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                     SWP_FRAMECHANGED | SWP_NOACTIVATE);

        ShowWindow(m_hwnd, SW_MAXIMIZE);
    }
    else
    {
        // Restore all the window decorators.
        SetWindowLong(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

        SetWindowPos(m_hwnd, HWND_NOTOPMOST, m_windowRect.left,
                     m_windowRect.top, m_windowRect.right - m_windowRect.left,
                     m_windowRect.bottom - m_windowRect.top,
                     SWP_FRAMECHANGED | SWP_NOACTIVATE);

        ShowWindow(m_hwnd, SW_NORMAL);
    }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void IWindowImpl::attach_toDestroy(Callback<void>& a_onDestroyCallback)
{
    m_signalWindowDestroyed.attach_ToSignal(a_onDestroyCallback);
}

}  // namespace m::win32