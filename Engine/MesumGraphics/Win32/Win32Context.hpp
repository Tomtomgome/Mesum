#ifndef M_WIN32CONTEXT
#define M_WIN32CONTEXT
#pragma once
#include <wrl.h>

#include <MesumCore/Kernel/Input.hpp>
#include <MesumCore/Kernel/Logger.hpp>
#include <MesumCore/Kernel/Types.hpp>

namespace m::win32
{
extern const logging::mChannelID PLATFORM_APP_ID;

struct Win32Context
{
    void init(HINSTANCE& a_hInstance);
    void init_keysLuts();

    void destroy();

    // Windows
    void register_windowClass(const mWideChar* a_className,
                              HINSTANCE a_hInstance, WNDPROC a_proc);
    HWND create_window(const mWideChar* a_className, std::string a_windowName,
                       mU32 a_clientWidth, mU32 a_clientHeight,
                       mBool a_isTransparent) const;

    // Keys and inputs
    [[nodiscard]] input::mKey get_keyFromParam(WPARAM a_wParam) const;

    // Properties
    input::mKey m_lut_keycodes[256];
    mI16        m_lut_scancode[input::mKey::keyLast + 1];

    HINSTANCE m_hInstance;
};

}  // namespace m::win32
#endif  // M_WIN32CONTEXT