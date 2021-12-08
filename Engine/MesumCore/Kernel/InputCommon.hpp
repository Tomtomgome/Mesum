#pragma once

#include "Callbacks.hpp"
#include "Logger.hpp"
#include "MathTypes.hpp"
#include "Types.hpp"

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

using mKeyActionCallback = mCallback<void>;  //!< Alias a keyaction callback
using mKeyActionSignal   = mSignal<>;        //!< Alias a keyaction signal

using mMouseActionCallback =
    mCallback<void, const math::mIVec2&>;  //!< Alias a mMouseActionCallback
using mMouseActionSignal =
    mSignal<const math::mIVec2&>;  //!< Alias a mMouseActionSignal

using mMouseStateCallback =
    mCallback<void, mBool>;                //!< Alias a mMouseStateCallback
using mMouseStateSignal = mSignal<mBool>;  //!< Alias a mMouseActionSignal

using mScrollCallback = mCallback<void, mDouble>;  //!< Alias a mScrollCallback
using mScrollSignal   = mSignal<mDouble>;          //!< Alias a mScrollSignal

using mMouseMoveCallback =
    mCallback<void, const math::mIVec2&>;  //!< Alias a mMouseMoveCallback
using mMouseMoveSignal =
    mSignal<const math::mIVec2&>;  //!< Alias a mMouseMoveSignal
}  // namespace m::input

///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////