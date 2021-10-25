#include <WindowsImplXCB.hpp>
#include <XCBContext.hpp>

#include <imgui.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND   hWnd,
                                                             UINT   msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);
namespace m
{
namespace xcb_unix
{

LRESULT IWindowImpl::process_messages(UINT a_uMsg, WPARAM a_wParam,
                                         LPARAM a_lParam)
{
    if (m_isImGuiWindow)
    {
        if (ImGui_ImplWin32_WndProcHandler(m_hwnd, a_uMsg, a_wParam, a_lParam))
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

            m_renderSurface->resize(width, height);
        }
        break;
        default: result = DefWindowProcW(m_hwnd, a_uMsg, a_wParam, a_lParam);
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

    if (m_parentRenderer != nullptr)
    {
        m_renderSurface = m_parentRenderer->get_newSurface();
        render::Win32SurfaceInitData surfaceData = {m_hwnd, m_clientWidth,
                                                    m_clientHeight};
        m_renderSurface->init_win32(surfaceData);
    }

    ShowWindow(m_hwnd, SW_NORMAL);
}

void IWindowImpl::render()
{
    if (m_renderSurface != nullptr)
    {
        m_renderSurface->render();
    }
}

void IWindowImpl::destroy()
{
    if (m_renderSurface != nullptr)
    {
        m_renderSurface->destroy();
        delete m_renderSurface;
        m_renderSurface = nullptr;
    }
    m_parentRenderer = nullptr;
    ::DestroyWindow(m_hwnd);
    m_hwnd = NULL;

    if (m_isImGuiWindow)
    {
        //ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}

void IWindowImpl::set_asMainWindow()
{
    static m::Bool s_mainWindowIsDefined = false;

    //There can only be one main window
    mAssert(s_mainWindowIsDefined == false);
    mAssert(m_isMainWindow == false);
    s_mainWindowIsDefined = true;
    m_isMainWindow = true;
}

void IWindowImpl::set_asImGuiWindow(Bool a_supportMultiViewports)
{
    // There can only be one ImGui window, and it's the main one
    mAssert(m_isMainWindow == true);
    mAssert(m_isImGuiWindow == false);
    m_isImGuiWindow = true;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    if (a_supportMultiViewports)
    {
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    }
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(m_hwnd);
    if (m_renderSurface != nullptr)
    {
        m_renderSurface->init_dearImGui(
            Callback<void>(this, &IWindowImpl::callback_dearImGuiNewFrame));
    }
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

void IWindowImpl::callback_dearImGuiNewFrame()
{
    //ImGui_ImplWin32_NewFrame();
}

} // namespace xcb_unix
}