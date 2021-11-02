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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
KeyAction KeyAction::keyPressed(mKey a_key, mKeyMod a_keyMod)
{
    return {mInputType::pressed, a_keyMod, a_key};
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
KeyAction KeyAction::keyReleased(mKey a_key, mKeyMod a_keyMod)
{
    return {mInputType::released, a_keyMod, a_key};
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InputManager::update_KeyMods(mKeyMod a_mod, mInputType a_action)
{
    if (a_action == mInputType::pressed &&
        (m_currentMod & mInt(a_mod)) != mInt(a_mod))
    {
        m_currentMod = m_currentMod | mInt(a_mod);
    }
    else if (a_action == mInputType::released &&
             (m_currentMod & mInt(a_mod)) == mInt(a_mod))
    {
        m_currentMod = m_currentMod & !(mInt(a_mod));
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InputManager::process_MouseEvent(mMouseButton a_button,
                                      mInputType a_action, mKeyMod a_mods)
{
    m_mouseSignals[{a_action, a_mods, a_button}].call(
        math::mDVec2({m_mousePosX, m_mousePosX}));
    m_mouseButtonClicked[mInt(a_button)] = a_action == mInputType::pressed;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InputManager::process_KeyEvent(mKey a_key, mInt a_scancode,
                                    mInputType a_action, mKeyMod a_mods)
{
    m_keySignals[{a_action, a_mods, a_key}].call();

    if (a_key == mKey::keyLeftControl || a_key == mKey::keyRightControl)
    {
        update_KeyMods(mKeyMod::ctrl, a_action);
    }
    else if (a_key == mKey::keyLeftAlt || a_key == mKey::keyRightAlt)
    {
        update_KeyMods(mKeyMod::alt, a_action);
    }
    else if (a_key == mKey::keyLeftShift || a_key == mKey::keyRightShift)
    {
        update_KeyMods(mKeyMod::shift, a_action);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InputManager::process_ScrollEvent(double a_xoffset, double a_yoffset)
{
    m_scrollSignal.call(math::mDVec2({a_xoffset, a_yoffset}));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InputManager::process_CursorPosition(double a_xpos, double a_ypos)
{
    m_moveSignal.call(
        math::mDVec2({a_xpos - m_mousePosX, a_ypos - m_mousePosY}));
    m_mousePosX = a_xpos;
    m_mousePosY = a_ypos;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InputManager::processAndUpdate_States()
{
    for (auto it = m_mouseStateSignals.begin(); it != m_mouseStateSignals.end();
         ++it)
    {
        MouseState state = it->first;
        if ((mInt(state.m_keyMod) & (m_currentMod ^ m_previousMod)) != 0 ||
            m_mouseButtonClicked[int(state.m_button)] !=
                m_previousMouseButtonClicked[mInt(state.m_button)])
        {
            it->second.call(((mInt(state.m_keyMod) & m_currentMod) ==
                             mInt(state.m_keyMod)) &&
                            m_mouseButtonClicked[mInt(state.m_button)]);
        }
    }

    m_previousMod                = m_currentMod;
    m_previousMouseButtonClicked = m_mouseButtonClicked;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mKeyActionSignal::mCallbackHandle InputManager::attach_ToKeyEvent(
    KeyAction a_keyAction, const mKeyActionCallback& a_callback)
{
    return m_keySignals[a_keyAction].attach_toSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mMouseActionSignal::mCallbackHandle InputManager::attach_ToMouseEvent(
    mMouseAction a_mouseAction, const mMouseActionCallback& a_callback)
{
    return m_mouseSignals[a_mouseAction].attach_toSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mScrollSignal::mCallbackHandle InputManager::attach_ToScrollEvent(
    const mScrollCallback& a_callback)
{
    return m_scrollSignal.attach_toSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mMouseMoveSignal::mCallbackHandle InputManager::attach_ToMoveEvent(
    const mMouseMoveCallback& a_callback)
{
    return m_moveSignal.attach_toSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InputManager::detach_FromKeyEvent(KeyAction                a_keyAction,
                                       const mKeyActionCallback& a_callback)
{
    m_keySignals[a_keyAction].detach_fromSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InputManager::detach_FromKeyEvent(
    KeyAction a_keyAction, const mKeyActionSignal::mCallbackHandle& a_handle)
{
    m_keySignals[a_keyAction].detach_fromSignal(a_handle);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InputManager::detach_FromMouseEvent(mMouseAction a_mouseAction,
                                         const mMouseActionCallback& a_callback)
{
    m_mouseSignals[a_mouseAction].detach_fromSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InputManager::detach_FromMouseEvent(
    mMouseAction                              a_mouseAction,
    const mMouseActionSignal::mCallbackHandle& a_handle)
{
    m_mouseSignals[a_mouseAction].detach_fromSignal(a_handle);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InputManager::detach_FromScrollEvent(const mScrollCallback& a_callback)
{
    m_scrollSignal.detach_fromSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InputManager::detach_FromScrollEvent(
    const mScrollSignal::mCallbackHandle& a_handle)
{
    m_scrollSignal.detach_fromSignal(a_handle);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InputManager::detach_FromMoveEvent(const mMouseMoveCallback& a_callback)
{
    m_moveSignal.detach_fromSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InputManager::detach_FromMoveEvent(
    const mMouseMoveSignal::mCallbackHandle& a_handle)
{
    m_moveSignal.detach_fromSignal(a_handle);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mMouseStateSignal::mCallbackHandle InputManager::attach_ToMouseState(
    MouseState a_mouseState, const mMouseStateCallback& a_callback)
{
    return m_mouseStateSignals[a_mouseState].attach_toSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InputManager::detach_FromMouseState(MouseState                a_mouseState,
                                         const mMouseStateCallback& a_callback)
{
    m_mouseStateSignals[a_mouseState].detach_fromSignal(a_callback);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void InputManager::detach_FromMouseState(
    MouseState a_mouseState, const mMouseStateSignal::mCallbackHandle& a_handle)
{
    m_mouseStateSignals[a_mouseState].detach_fromSignal(a_handle);
}
};  // namespace m::input