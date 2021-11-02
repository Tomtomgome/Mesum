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
extern const logging::mChannelID PLATFORM_APP_ID;

struct XCBContext
{
    void init();
    void init_keysLuts();

    void destroy();

    // Properties
    input::mKey m_lut_keycodes[256];
    mI16       m_lut_scancode[input::mKey::keyLast + 1];

};

}  // namespace xcb_unix
}  // namespace m
#endif  // M_XCBCONTEXT