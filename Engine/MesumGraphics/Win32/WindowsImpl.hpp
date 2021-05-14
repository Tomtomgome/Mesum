#ifndef M_WINDOWSIMPL
#define M_WINDOWSIMPL
#pragma once

#include <MesumCore/Kernel/Types.hpp>
#include <MesumGraphics/CrossRenderer.hpp>
#include <MesumGraphics/Windows.hpp>

namespace m::win32
{
class Win32Context;

class IWindowImpl : public windows::IWindow
{
   public:
    // Platform specific
    LRESULT process_messages(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam);

    void init() override;
    void destroy() override;

    render::ISurface* get_renderSurface() override { return m_surfaceHandle->surface; }

    void link_inputManager(input::InputManager* a_inputManager) override
    {
        m_linkedInputManager = a_inputManager;
    };
    render::ISurface::HdlPtr link_renderer(
        render::IRenderer* a_renderer) override;
    void set_size(UInt a_width, UInt a_height) override
    {
        m_clientWidth  = a_width;
        m_clientHeight = a_height;
    }
    void set_windowName(std::string a_name) override { m_windowName = a_name; }
    void set_asMainWindow() override;
    void set_asImGuiWindow(Bool a_supportMultiViewports) override;
    void set_fullScreen(Bool a_fullscreen) override;
    void toggle_fullScreen() override;

    void set_winContext(WIN32Context const& a_winContext)
    {
        m_parentContext = &a_winContext;
    }

    [[nodiscard]] Bool is_flaggedToBeClosed() const { return m_flagToBeClosed; }

   private:
    input::InputManager* m_linkedInputManager;

    HWND m_hwnd;

    // By default, use windowed mode.
    // Can be toggled with F11
    Bool m_fullscreen     = false;
    Bool m_flagToBeClosed = false;

    std::string m_windowName;
    Bool        m_isMainWindow  = false;
    Bool        m_isImGuiWindow = false;
    U32         m_clientWidth   = 1280;
    U32         m_clientHeight  = 720;

    // Window rectangle (used to toggle fullscreen state).
    RECT m_windowRect;

    WIN32Context const*      m_parentContext = nullptr;
    render::ISurface::HdlPtr m_surfaceHandle = nullptr;

    Signal<U32, U32> m_resizeSignal;
};

}  // namespace m::win32
#endif  // M_WINDOWSIMPL