#include <WindowsImpl.hpp>
#include <Win32Context.hpp>

namespace m
{
namespace win32
{
LRESULT IWindowImpl::process_messages(UINT a_uMsg, WPARAM a_wParam,
                                         LPARAM a_lParam)
{
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

        case WM_DESTROY: PostQuitMessage(0); break;

        case WM_SIZE:
        {
            RECT clientRect = {};
            ::GetClientRect(m_hwnd, &clientRect);

            U32 width  = clientRect.right - clientRect.left;
            U32 height = clientRect.bottom - clientRect.top;

            m_window.resize(width, height);
        }
        break;
        default: result = DefWindowProc(m_hwnd, a_uMsg, a_wParam, a_lParam);
    }
    return result;
}

void IWindowImpl::init()
{
    const Char className[] = L"MainWindowClass";
    m_hwnd = m_parentContext->create_window(className, m_windowName,
                                           m_clientWidth,
                                        m_clientHeight);
    GetWindowRect(m_hwnd, &m_windowRect);

    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, LONG_PTR(this));

    m_window.init(m_hwnd, m_clientWidth, m_clientHeight);

    ShowWindow(m_hwnd, SW_NORMAL);
}

void IWindowImpl::render()
{
    m_window.render();
}

void IWindowImpl::destroy()
{
    m_window.destroy();
}

void IWindowImpl::set_fullScreen(Bool a_fullscreen)
{
    if (m_fullscreen != a_fullscreen)
    {
        toggle_fullScreen();
    }
}

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
}
}