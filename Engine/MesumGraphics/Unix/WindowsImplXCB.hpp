#ifndef M_WINDOWSIMPLXCB
#define M_WINDOWSIMPLXCB
#pragma once

#include <MesumCore/Kernel/Types.hpp>
#include <MesumGraphics/CrossRenderer.hpp>
#include <MesumGraphics/Windows.hpp>

namespace m
{
namespace xcb_unix
{
class XCBContext;

class IWindowImpl : public windows::IWindow
{
   public:
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

    void set_winContext(XCBContext const& a_xcbContext)
    {
        m_parentContext = &a_xcbContext;
    }

    Bool is_flaggedToBeClosed() { return m_flagToBeClosed; }

    void callback_dearImGuiNewFrame();

   private:
    input::InputManager* m_linkedInputManager;

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
    m::math::UIVec4 m_windowRect;

    XCBContext const* m_parentContext  = nullptr;
    render::IRenderer*  m_parentRenderer = nullptr;
    render::ISurface*   m_renderSurface  = nullptr;
};

}  // namespace xcb_unix
}  // namespace m
#endif  // M_WINDOWSIMPLXCB