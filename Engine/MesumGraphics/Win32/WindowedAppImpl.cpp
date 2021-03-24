#include <WindowedAppImpl.hpp>
#include <WindowsImpl.hpp>
#include <DX12Renderer.hpp>

namespace m
{
namespace win32
{

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

windows::IWindow* IWindowedApplicationImpl::add_newWindow(std::wstring a_name,
                                                          U32          a_width,
                                                          U32          a_height)
{
    IWindowImpl* newWindow = new IWindowImpl();
    m_windows.insert(newWindow);

    newWindow->set_size(a_width, a_height);
    newWindow->set_windowName(a_name);
    newWindow->set_winContext(m_W32Context);
    newWindow->init();

    return newWindow;
}

void IWindowedApplicationImpl::init()
{
    WindowedLaunchData& data = *(WindowedLaunchData*)m_appData;

    m_W32Context.init(data.m_hInstance);

    // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
    // Using this awareness context allows the client area of the window
    // to achieve 100% scaling while still allowing non-client window content to
    // be rendered in a DPI sensitive fashion.
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    const Char className[] = L"MainWindowClass";
    // Register the window class.
    m_W32Context.register_windowClass(className, data.m_hInstance, WindowProc);

    dx12::openRenderModule();
}

void IWindowedApplicationImpl::destroy()
{
    for (auto element = m_windows.begin(); element != m_windows.end();
         ++element)
    {
        windows::IWindow* window = (*element);
        window->destroy();
        delete window;
    }
    m_windows.clear();

    dx12::closeRenderModule();

    m_W32Context.destroy();
}

Bool IWindowedApplicationImpl::step(const Double& a_deltaTime)
{
    Bool signalKeepRunning = true;

    // Run the message loop.
    MSG msg = {};
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            signalKeepRunning = false;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    auto element = m_windows.begin();
    while (element != m_windows.end())
    {
        windows::IWindow* window = (*element);
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

    for (auto element = m_windows.begin(); element != m_windows.end();
         ++element)
    {
        windows::IWindow* window = (*element);
        window->render();
    }

    return signalKeepRunning;
}
}  // namespace platWindows
};  // namespace m