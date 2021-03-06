#ifndef M_WIN32CONTEXT
#define M_WIN32CONTEXT
#pragma once
#include <wrl.h>

#include <MesumCore/Kernel/Types.hpp>
#include <MesumCore/Kernel/Input.hpp>
#include <MesumCore/Kernel/Logger.hpp>

namespace m
{
namespace win32
{
extern const logging::ChannelID PLATFORM_APP_ID;

struct WIN32Context
{
    void init(HINSTANCE& a_hInstance);
    void init_keysLuts();

    void destroy();

    // Windows
    void register_windowClass(const Char* a_className, HINSTANCE a_hInstance,
                              WNDPROC a_proc);
    HWND create_window(const Char* a_className, std::wstring a_windowName,
                       U32 a_clientWidth, U32 a_clientHeight) const;

    // Keys and inputs
    input::Key get_keyFromParam(WPARAM a_wParam) const;

    // Properties
    input::Key m_lut_keycodes[256];
    I16        m_lut_scancode[input::Key::KEY_LAST + 1];

    HINSTANCE m_hInstance;
};

}  // namespace win32
}  // namespace m
#endif  // M_WIN32CONTEXT