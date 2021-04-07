#ifndef M_XCBCONTEXT
#define M_XCBCONTEXT
#pragma once

#include <MesumCore/Kernel/Types.hpp>
#include <MesumCore/Kernel/Input.hpp>
#include <MesumCore/Kernel/Logger.hpp>

namespace m
{
namespace xcb_unix
{
extern const logging::ChannelID PLATFORM_APP_ID;

struct XCBContext
{
    void init();
    void init_keysLuts();

    void destroy();

    // Properties
    input::Key m_lut_keycodes[256];
    I16        m_lut_scancode[input::Key::KEY_LAST + 1];

};

}  // namespace xcb_unix
}  // namespace m
#endif  // M_XCBCONTEXT