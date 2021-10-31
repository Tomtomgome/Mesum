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
using KeyActionCallback = mCallback<void>;
using KeyActionSignal   = mSignal<>;

using MouseActionCallback = mCallback<void, const math::DVec2&>;
using MouseActionSignal   = mSignal<const math::DVec2&>;

using MouseStateCallback = mCallback<void, Bool>;
using MouseStateSignal   = mSignal<Bool>;

using ScrollCallback = mCallback<void, const math::DVec2&>;
using ScrollSignal   = mSignal<const math::DVec2&>;

using MouseMoveCallback = mCallback<void, const math::DVec2&>;
using MouseMoveSignal   = mSignal<const math::DVec2&>;
}  // namespace input
}  // namespace m

#endif //M_INPUTCOMMON