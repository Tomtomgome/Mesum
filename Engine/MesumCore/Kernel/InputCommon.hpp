#ifndef M_INPUTCOMMON
#define M_INPUTCOMMON
#pragma once

#include <Logger.hpp>
#include <Callbacks.hpp>
#include <Types.hpp>
#include <MathTypes.hpp>

namespace m
{
extern const logging::ChannelID INPUT_LOG_ID;

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
namespace input
{
using KeyActionCallback = Callback<void>;
using KeyActionSignal   = Signal<>;

using MouseActionCallback = Callback<void, const math::DVec2&>;
using MouseActionSignal   = Signal<const math::DVec2&>;

using MouseStateCallback = Callback<void, Bool>;
using MouseStateSignal   = Signal<Bool>;

using ScrollCallback = Callback<void, const math::DVec2&>;
using ScrollSignal   = Signal<const math::DVec2&>;

using MouseMoveCallback = Callback<void, const math::DVec2&>;
using MouseMoveSignal   = Signal<const math::DVec2&>;
}  // namespace input
}  // namespace m

#endif //M_INPUTCOMMON