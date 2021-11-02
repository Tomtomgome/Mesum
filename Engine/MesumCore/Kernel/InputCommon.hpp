#pragma once

#include <Callbacks.hpp>
#include <Logger.hpp>
#include <MathTypes.hpp>
#include <Types.hpp>

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// \brief Namespace grouping basic input management
///////////////////////////////////////////////////////////////////////////////
namespace m::input
{
extern MesumCoreApi const logging::mChannelID g_inputLogID;

using mKeyActionCallback = mCallback<void>;
using mKeyActionSignal   = mSignal<>;

using mMouseActionCallback = mCallback<void, const math::mDVec2&>;
using mMouseActionSignal   = mSignal<const math::mDVec2&>;

using mMouseStateCallback = mCallback<void, mBool>;
using mMouseStateSignal   = mSignal<mBool>;

using mScrollCallback = mCallback<void, const math::mDVec2&>;
using mScrollSignal   = mSignal<const math::mDVec2&>;

using mMouseMoveCallback = mCallback<void, const math::mDVec2&>;
using mMouseMoveSignal   = mSignal<const math::mDVec2&>;
}  // namespace m::input

///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////