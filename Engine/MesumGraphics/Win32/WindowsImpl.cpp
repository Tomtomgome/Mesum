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

    mBool processed = true;
    if (m_linkedInputManager == nullptr)
    {
        processed = false;
    }
    else
    {
        switch (a_uMsg)
        {
            case WM_KEYDOWN:
            {
                input::mKey k = m_parentContext->get_keyFromParam(a_wParam);

                m_linkedInputManager->process_keyEvent(
                    k, input::mInputType::pressed);
            }
            break;
            case WM_KEYUP:
            {
                input::mKey k = m_parentContext->get_keyFromParam(a_wParam);

                m_linkedInputManager->process_keyEvent(
                    k, input::mInputType::released);
            }
            break;

            case WM_LBUTTONDOWN:
            {
                m_linkedInputManager->process_mouseEvent(
                    input::mMouseButton::left, input::mInputType::pressed);
            }
            break;
            case WM_LBUTTONUP:
            {
                m_linkedInputManager->process_mouseEvent(
                    input::mMouseButton::left, input::mInputType::released);
            }
            break;

            case WM_RBUTTONDOWN:
            {
                m_linkedInputManager->process_mouseEvent(
                    input::mMouseButton::right, input::mInputType::pressed);
            }
            break;
            case WM_RBUTTONUP:
            {
                m_linkedInputManager->process_mouseEvent(
                    input::mMouseButton::right, input::mInputType::released);
            }
            break;

            case WM_MBUTTONDOWN:
            {
                m_linkedInputManager->process_mouseEvent(
                    input::mMouseButton::middle, input::mInputType::pressed);
            }
            break;
            case WM_MBUTTONUP:
            {
                m_linkedInputManager->process_mouseEvent(
                    input::mMouseButton::middle, input::mInputType::released);
            }
            break;
            case WM_MOUSEMOVE:
            {
                mInt x = static_cast<mI16>(LOWORD(a_lParam));
                mInt y = static_cast<mI16>(HIWORD(a_lParam));
                m_linkedInputManager->process_mouseMoveEvent(x, y);
            }
            break;
            case WM_MOUSEWHEEL:
            {
                m_linkedInputManager->process_mouseScrollEvent(
                    (SHORT)HIWORD(a_wParam) / (double)WHEEL_DELTA);
            }
            break;
            default: processed = false;
        }
    }

    switch (a_uMsg)
    {
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

            mU32 width  = clientRect.right - clientRect.left;
            mU32 height = clientRect.bottom - clientRect.top;

            m_signalResize.call(width, height);
        }
        break;
        default:
        {
            if (!processed)
            {
                result = DefWindowProcW(m_hwnd, a_uMsg, a_wParam, a_lParam);
            }
        }
    }
    return result;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void IWindowImpl::init()
{
    const mWideChar className[] = L"MainWindowClass";
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
    mAssert(a_renderer != nullptr);
    render::ISurface::HdlPtr surfaceHandle =
        std::make_shared<render::ISurface::Handle>();
    surfaceHandle->surface                   = a_renderer->get_newSurface();
    render::Win32SurfaceInitData surfaceData = {m_hwnd, m_clientWidth,
                                                m_clientHeight};
    surfaceHandle->surface->init_win32(surfaceData);

    m_signalResize.attach_toSignal(mCallback<void, mU32, mU32>(
        surfaceHandle->surface, &render::ISurface::resize));

    // TODO : Manage this from the renderer
    m_signalWindowDestroyed.attach_toSignal(mCallback<void>(
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
    static m::mBool s_mainWindowIsDefined = false;

    // There can only be one main window
    mAssert(s_mainWindowIsDefined == false);
    mAssert(m_isMainWindow == false);
    s_mainWindowIsDefined = true;
    m_isMainWindow        = true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void IWindowImpl::set_asImGuiWindow()
{
    // There can only be one ImGui window, and it's the main one
    mAssert(m_isMainWindow == true);

    ImGui_ImplWin32_Init(m_hwnd);

    m_signalWindowDestroyed.attach_toSignal(
        mCallback<void>([]() { ImGui_ImplWin32_Shutdown(); }));

    m_signalOverrideInputProcessing.attach_toSignal(CallbackInputProcessing(
        [](mBool* a_interrupt, HWND a_hwnd, UINT a_uMsg, WPARAM a_wParam,
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
void IWindowImpl::set_fullScreen(mBool a_fullscreen)
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
void IWindowImpl::attach_toDestroy(mCallback<void> const& a_onDestroyCallback)
{
    m_signalWindowDestroyed.attach_toSignal(a_onDestroyCallback);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void IWindowImpl::attach_toResize(
    mCallback<void, mU32, mU32> const& a_onResizeCallback)
{
    m_signalResize.attach_toSignal(a_onResizeCallback);
}

}  // namespace m::win32