#ifndef M_WINDOWSIMPL
#define M_WINDOWSIMPL
#pragma once

#include <MesumCore/Kernel/Types.hpp>
#include <MesumGraphics/CrossRenderer.hpp>
#include <MesumGraphics/Windows.hpp>

namespace m
{
namespace win32
{
class Win32Context;

class IWindowImpl : public windows::IWindow
{
   public:
    // Platform specific
    LRESULT process_messages(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam);

    virtual void init() override;
    virtual void render() override;
    virtual void destroy() override;

    virtual void link_inputManager(input::InputManager* a_inputManager)
    {
        m_linkedInputManager = a_inputManager;
    };
    virtual void set_size(UInt a_width, UInt a_height)
    {
        m_clientWidth  = a_width;
        m_clientHeight = a_height;
    }
    virtual void set_windowName(std::wstring a_name) { m_windowName = a_name; }
    virtual void set_renderer(render::IRenderer* a_renderer)
    {
        m_parentRenderer = a_renderer;
    }
    virtual void set_asMainWindow();
    virtual void set_asImGuiWindow(Bool a_supportMultiViewports);
    virtual void set_fullScreen(Bool a_fullscreen);
    virtual void toggle_fullScreen();

    void set_winContext(WIN32Context const& a_winContext)
    {
        m_parentContext = &a_winContext;
    }

    Bool is_flaggedToBeClosed() { return m_flagToBeClosed; }

    void callback_dearImGuiNewFrame();

   private:
    input::InputManager* m_linkedInputManager;

    HWND m_hwnd;

    // By default, use windowed mode.
    // Can be toggled with F11
    Bool m_fullscreen     = false;
    Bool m_flagToBeClosed = false;

    std::wstring m_windowName;
    Bool         m_isMainWindow  = false;
    Bool         m_isImGuiWindow = false;
    U32          m_clientWidth   = 1280;
    U32          m_clientHeight  = 720;

    // Window rectangle (used to toggle fullscreen state).
    RECT m_windowRect;

    WIN32Context const* m_parentContext  = nullptr;
    render::IRenderer*  m_parentRenderer = nullptr;
    render::ISurface*   m_renderSurface  = nullptr;
};

}  // namespace win32
}  // namespace m
#endif  // M_WINDOWSIMPL