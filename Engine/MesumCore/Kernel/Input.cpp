#include <Input.hpp>

namespace m::input
{
MesumCoreApi const logging::mChannelID g_inputLogID = mLog_getId();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mMouseAction mMouseAction::mousePressed(mMouseButton a_button, mKeyMod a_keyMod)
{
    return {mInputType::pressed, a_keyMod, a_button};
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mMouseAction mMouseAction::mouseReleased(mMouseButton a_button,
                                         mKeyMod      a_keyMod)
{
    return {mInputType::released, a_keyMod, a_button};
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mMouseAction::mMouseAction(mInputType a_action, mKeyMod a_keyMod,
                           mMouseButton a_button)
    : m_action(a_action),
      m_keyMod(a_keyMod),
      m_button(a_button)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mKeyAction mKeyAction::keyPressed(mKey a_key, mKeyMod a_keyMod)
{
    return {mInputType::pressed, a_keyMod, a_key};
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mKeyAction mKeyAction::keyReleased(mKey a_key, mKeyMod a_keyMod)
{
    return {mInputType::released, a_keyMod, a_key};
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mKeyAction::mKeyAction(mInputType a_action, mKeyMod a_keyMod, mKey a_key)
    : m_action(a_action),
      m_keyMod(a_keyMod),
      m_key(a_key)
{
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCallbackInputManager::process_keyEvent(mKey a_key, mInputType a_action)
{
    keySignals[{a_action, currentKeyMod, a_key}].call();

    if (a_key == mKey::keyLeftControl || a_key == mKey::keyRightControl)
    {
        update_keyMods(mKeyMod::ctrl, a_action);
    }
    else if (a_key == mKey::keyLeftAlt || a_key == mKey::keyRightAlt)
    {
        update_keyMods(mKeyMod::alt, a_action);
    }
    else if (a_key == mKey::keyLeftShift || a_key == mKey::keyRightShift)
    {
        update_keyMods(mKeyMod::shift, a_action);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCallbackInputManager::process_mouseEvent(mMouseButton a_button,
                                               mInputType   a_action)
{
    mouseSignals[{a_action, currentKeyMod, a_button}].call(currentMousePos);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCallbackInputManager::process_mouseMoveEvent(mInt a_xPos, mInt a_yPos)
{
    math::mIVec2 newMousePos({a_xPos, a_yPos});
    moveSignal.call(newMousePos - currentMousePos);
    currentMousePos = newMousePos;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCallbackInputManager::process_mouseScrollEvent(mDouble a_offset)
{
    scrollSignal.call(a_offset);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCallbackInputManager::update_keyMods(mKeyMod a_mod, mInputType a_action)
{
    if (a_action == mInputType::pressed && (currentKeyMod & a_mod) != a_mod)
    {
        currentKeyMod = currentKeyMod | a_mod;
    }
    else if (a_action == mInputType::released &&
             (currentKeyMod & a_mod) == a_mod)
    {
        currentKeyMod = currentKeyMod & !a_mod;
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mKeyActionSignal::mCallbackHandle mCallbackInputManager::attach_toKeyEvent(
    mKeyAction a_keyAction, const mKeyActionCallback& a_callback)
{
    return keySignals[a_keyAction].attach_toSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mMouseActionSignal::mCallbackHandle mCallbackInputManager::attach_toMouseEvent(
    mMouseAction a_mouseAction, const mMouseActionCallback& a_callback)
{
    return mouseSignals[a_mouseAction].attach_toSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mMouseMoveSignal::mCallbackHandle
mCallbackInputManager::attach_toMouseMoveEvent(
    const mMouseMoveCallback& a_callback)
{
    return moveSignal.attach_toSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mScrollSignal::mCallbackHandle mCallbackInputManager::attach_toMouseScrollEvent(
    const mScrollCallback& a_callback)
{
    return scrollSignal.attach_toSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCallbackInputManager::detach_fromKeyEvent(
    mKeyAction a_keyAction, const mKeyActionCallback& a_callback)
{
    keySignals[a_keyAction].detach_fromSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCallbackInputManager::detach_fromKeyEvent(
    mKeyAction a_keyAction, const mKeyActionSignal::mCallbackHandle& a_handle)
{
    keySignals[a_keyAction].detach_fromSignal(a_handle);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCallbackInputManager::detach_fromMouseEvent(
    mMouseAction a_mouseAction, const mMouseActionCallback& a_callback)
{
    mouseSignals[a_mouseAction].detach_fromSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCallbackInputManager::detach_fromMouseEvent(
    mMouseAction                               a_mouseAction,
    const mMouseActionSignal::mCallbackHandle& a_handle)
{
    mouseSignals[a_mouseAction].detach_fromSignal(a_handle);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCallbackInputManager::detach_fromMouseScrollEvent(
    const mScrollCallback& a_callback)
{
    scrollSignal.detach_fromSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCallbackInputManager::detach_fromMouseScrollEvent(
    const mScrollSignal::mCallbackHandle& a_handle)
{
    scrollSignal.detach_fromSignal(a_handle);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCallbackInputManager::detach_fromMouseMoveEvent(
    const mMouseMoveCallback& a_callback)
{
    moveSignal.detach_fromSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCallbackInputManager::detach_fromMouseMoveEvent(
    const mMouseMoveSignal::mCallbackHandle& a_handle)
{
    moveSignal.detach_fromSignal(a_handle);
}
};  // namespace m::input