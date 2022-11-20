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

    void init(mInitData const& a_initData) final;
    void destroy() final;

    std::pair<mU32, mU32> get_size() const final;

    void link_inputManager(input::mIInputManager* a_inputManager) final
    {
        m_linkedInputManager = a_inputManager;
    };

    void set_asImGuiWindow() final;
    void set_fullScreen(mBool a_fullscreen) final;
    void toggle_fullScreen() final;

    void attach_toDestroy(mCallback<void> const& a_onDestroyCallback) final;
    void attach_toResize(
        mCallback<void, mU32, mU32> const& a_onResizeCallback) final;

    void set_winContext(Win32Context const& a_winContext)
    {
        m_parentContext = &a_winContext;
    }

    [[nodiscard]] mBool is_flaggedToBeClosed() const
    {
        return m_flagToBeClosed;
    }

    // Specific to win functions
    void attach_toSpecialUpdate(
        mCallback<void, std::chrono::steady_clock::duration const&> const&
            a_onUpdateCallback);
    HWND get_hwnd() { return m_hwnd; }
    void call_update(std::chrono::steady_clock::duration const& a_deltaTime);

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

    Win32Context const* m_parentContext = nullptr;

    using CallbackResize        = mCallback<void, mU32, mU32>;
    using CallbackWindowDestroy = mCallback<void>;
    using CallbackInputProcessing =
        mCallback<void, mBool*, HWND, UINT, WPARAM, LPARAM>;

    mSignal<std::chrono::steady_clock::duration const&> m_signalSpecialUpdate;

    mSignal<mU32, mU32>                         m_signalResize;
    mSignal<>                                   m_signalWindowDestroyed;
    mSignal<mBool*, HWND, UINT, WPARAM, LPARAM> m_signalOverrideInputProcessing;
};

}  // namespace m::win32
#endif  // M_WINDOWSIMPL