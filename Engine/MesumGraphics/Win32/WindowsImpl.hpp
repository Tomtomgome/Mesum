#ifndef M_WINDOWSIMPL
#define M_WINDOWSIMPL
#pragma once

#include <Kernel/Types.hpp>
#include "CrossRenderer.hpp"
#include "Windows.hpp"

namespace m::win32
{
class Win32Context;

class IWindowImpl : public windows::mIWindow
{
   public:
    // Platform specific
    LRESULT process_messages(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam);

    void init(mInitData const& a_initData) override;
    void destroy() override;

    void link_inputManager(input::mIInputManager* a_inputManager) override
    {
        m_linkedInputManager = a_inputManager;
    };
    render::ISurface::HdlPtr link_renderer(
        render::IRenderer* a_renderer) override;

    void set_asImGuiWindow() override;
    void set_fullScreen(mBool a_fullscreen) override;
    void toggle_fullScreen() override;

    void attach_toDestroy(mCallback<void> const& a_onDestroyCallback) override;
    void attach_toResize(
        mCallback<void, mU32, mU32> const& a_onResizeCallback) override;

    void set_winContext(WIN32Context const& a_winContext)
    {
        m_parentContext = &a_winContext;
    }

    [[nodiscard]] mBool is_flaggedToBeClosed() const
    {
        return m_flagToBeClosed;
    }

   private:
    input::mIInputManager* m_linkedInputManager;

    HWND m_hwnd;

    // By default, use windowed mode.
    // Can be toggled with F11
    mBool m_fullscreen     = false;
    mBool m_flagToBeClosed = false;

    std::string m_windowName;
    mU32        m_clientWidth  = 1280;
    mU32        m_clientHeight = 720;

    // Window rectangle (used to toggle fullscreen state).
    RECT m_windowRect;

    WIN32Context const* m_parentContext = nullptr;

    using CallbackResize        = mCallback<void, mU32, mU32>;
    using CallbackWindowDestroy = mCallback<void>;
    using CallbackInputProcessing =
        mCallback<void, mBool*, HWND, UINT, WPARAM, LPARAM>;
    mSignal<mU32, mU32>                         m_signalResize;
    mSignal<>                                   m_signalWindowDestroyed;
    mSignal<mBool*, HWND, UINT, WPARAM, LPARAM> m_signalOverrideInputProcessing;
};

}  // namespace m::win32
#endif  // M_WINDOWSIMPL