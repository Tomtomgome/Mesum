#include <DearImgui/imgui.h>

#include <CrossRenderer.hpp>
#include <WindowedAppImpl.hpp>
#include <WindowsImpl.hpp>

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

void IWindowedApplicationImpl::init_renderer(render::RendererApi a_renderApi)
{
    switch (a_renderApi)
    {
        case m::render::RendererApi::Default:
            m_renderer = new renderApi::DefaultRenderer();
            break;
        case m::render::RendererApi::DX12:
#if (defined M_DX12_RENDERER) || (defined M_ALL_RENDERER)
            m_renderer = new dx12::DX12Renderer();
#else
            //DX12Renderer Not Supported
            mInterrupt;
#endif
            break;
        case m::render::RendererApi::Vulkan:
#if (defined M_VULKAN_RENDERER) || (defined M_ALL_RENDERER)
            m_renderer = new vulkan::VulkanRenderer();
#else
            // VulkanRenderer Not Supported
            mInterrupt;
#endif
            //m_renderer = new vulkan::VulkanContext();
            break;
        default: mInterrupt; break;
    }
    m_renderer->init();
}

windows::IWindow* IWindowedApplicationImpl::add_newWindow(std::wstring a_name,
                                                          U32          a_width,
                                                          U32          a_height)
{
    IWindowImpl* newWindow = new IWindowImpl();
    m_windows.insert(newWindow);

    newWindow->set_size(a_width, a_height);
    newWindow->set_windowName(a_name);
    newWindow->set_renderer(m_renderer);
    newWindow->set_winContext(m_W32Context);
    newWindow->init();

    return newWindow;
}

void IWindowedApplicationImpl::set_processImGuiMultiViewports(
    Bool a_supportMultiViewPorts)
{
    mHardAssert((!a_supportMultiViewPorts) ||
                (m_renderer->get_supportDearImGuiMultiViewports() == true));
    m_supportImGuiMultiViewPorts = a_supportMultiViewPorts;
}

void IWindowedApplicationImpl::start_dearImGuiNewFrame() {
    mHardAssert(m_renderer != nullptr);
    m_renderer->start_dearImGuiNewFrame();
}


void IWindowedApplicationImpl::render()
{
    for (auto element = m_windows.begin(); element != m_windows.end();
         ++element)
    {
        windows::IWindow* window = (*element);
        window->render();
    }

    if (m_supportImGuiMultiViewPorts &&
        ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
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

    if (m_renderer != nullptr)
    {
        m_renderer->destroy();
        delete m_renderer;
    }

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

    if (m_windows.size() == 0)
    {
        signalKeepRunning = false;
    }

    return signalKeepRunning;
}
}  // namespace win32
};  // namespace m