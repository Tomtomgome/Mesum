#include <Input.hpp>

namespace m
{
extern const logging::mChannelID INPUT_LOG_ID = mLog_getId();

namespace input
{
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
MouseAction MouseAction::mousePressed(MouseButton a_button, KeyMod a_keyMod)
{
    return {Action::PRESSED, a_keyMod, a_button};
}
MouseAction MouseAction::mouseReleased(MouseButton a_button, KeyMod a_keyMod)
{
    return {Action::RELEASED, a_keyMod, a_button};
}

//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
KeyAction KeyAction::keyPressed(Key a_key, KeyMod a_keyMod)
{
    return {Action::PRESSED, a_keyMod, a_key};
}
KeyAction KeyAction::keyReleased(Key a_key, KeyMod a_keyMod)
{
    return {Action::RELEASED, a_keyMod, a_key};
}

//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
void InputManager::update_KeyMods(KeyMod a_mod, Action a_action)
{
    if (a_action == Action::PRESSED &&
        (m_currentMod & mInt(a_mod)) != mInt(a_mod))
    {
        m_currentMod = m_currentMod | mInt(a_mod);
    }
    else if (a_action == Action::RELEASED &&
             (m_currentMod & mInt(a_mod)) == mInt(a_mod))
    {
        m_currentMod = m_currentMod & !(mInt(a_mod));
    }
}

void InputManager::process_MouseEvent(MouseButton a_button, Action a_action,
                                      KeyMod a_mods)
{
    m_mouseSignals[{a_action, a_mods, a_button}].call(
        math::mDVec2({m_mousePosX, m_mousePosX}));
    m_mouseButtonClicked[mInt(a_button)] = a_action == Action::PRESSED;
}
void InputManager::process_KeyEvent(Key a_key, mInt a_scancode, Action a_action,
                                    KeyMod a_mods)
{
    m_keySignals[{a_action, a_mods, a_key}].call();

    if (a_key == Key::KEY_LEFT_CONTROL || a_key == Key::KEY_RIGHT_CONTROL)
    {
        update_KeyMods(KeyMod::CTRL, a_action);
    }
    else if (a_key == Key::KEY_LEFT_ALT || a_key == Key::KEY_RIGHT_ALT)
    {
        update_KeyMods(KeyMod::ALT, a_action);
    }
    else if (a_key == Key::KEY_LEFT_SHIFT || a_key == Key::KEY_RIGHT_SHIFT)
    {
        update_KeyMods(KeyMod::SHIFT, a_action);
    }
}
void InputManager::process_ScrollEvent(double a_xoffset, double a_yoffset)
{
    m_scrollSignal.call(math::mDVec2({a_xoffset, a_yoffset}));
}
void InputManager::process_CursorPosition(double a_xpos, double a_ypos)
{
    m_moveSignal.call(
        math::mDVec2({a_xpos - m_mousePosX, a_ypos - m_mousePosY}));
    m_mousePosX = a_xpos;
    m_mousePosY = a_ypos;
}

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
            it->second.call(
                ((mInt(state.m_keyMod) & m_currentMod) ==
                             mInt(state.m_keyMod)) &&
                m_mouseButtonClicked[mInt(state.m_button)]);
        }
    }

    m_previousMod                = m_currentMod;
    m_previousMouseButtonClicked = m_mouseButtonClicked;
}

KeyActionSignal::mCallbackHandle InputManager::attach_ToKeyEvent(
    KeyAction a_keyAction, const KeyActionCallback& a_callback)
{
    return m_keySignals[a_keyAction].attach_toSignal(a_callback);
}
MouseActionSignal::mCallbackHandle InputManager::attach_ToMouseEvent(
    MouseAction a_mouseAction, const MouseActionCallback& a_callback)
{
    return m_mouseSignals[a_mouseAction].attach_toSignal(a_callback);
}
ScrollSignal::mCallbackHandle InputManager::attach_ToScrollEvent(
    const ScrollCallback& a_callback)
{
    return m_scrollSignal.attach_toSignal(a_callback);
}
MouseMoveSignal::mCallbackHandle InputManager::attach_ToMoveEvent(
    const MouseMoveCallback& a_callback)
{
    return m_moveSignal.attach_toSignal(a_callback);
}

void InputManager::detach_FromKeyEvent(KeyAction                a_keyAction,
                                       const KeyActionCallback& a_callback)
{
    m_keySignals[a_keyAction].detach_fromSignal(a_callback);
}
void InputManager::detach_FromKeyEvent(
    KeyAction a_keyAction, const KeyActionSignal::mCallbackHandle& a_handle)
{
    m_keySignals[a_keyAction].detach_fromSignal(a_handle);
}

void InputManager::detach_FromMouseEvent(MouseAction a_mouseAction,
                                         const MouseActionCallback& a_callback)
{
    m_mouseSignals[a_mouseAction].detach_fromSignal(a_callback);
}
void InputManager::detach_FromMouseEvent(
    MouseAction                               a_mouseAction,
    const MouseActionSignal::mCallbackHandle& a_handle)
{
    m_mouseSignals[a_mouseAction].detach_fromSignal(a_handle);
}

void InputManager::detach_FromScrollEvent(const ScrollCallback& a_callback)
{
    m_scrollSignal.detach_fromSignal(a_callback);
}
void InputManager::detach_FromScrollEvent(
    const ScrollSignal::mCallbackHandle& a_handle)
{
    m_scrollSignal.detach_fromSignal(a_handle);
}

void InputManager::detach_FromMoveEvent(const MouseMoveCallback& a_callback)
{
    m_moveSignal.detach_fromSignal(a_callback);
}
void InputManager::detach_FromMoveEvent(
    const MouseMoveSignal::mCallbackHandle& a_handle)
{
    m_moveSignal.detach_fromSignal(a_handle);
}

MouseStateSignal::mCallbackHandle InputManager::attach_ToMouseState(
    MouseState a_mouseState, const MouseStateCallback& a_callback)
{
    return m_mouseStateSignals[a_mouseState].attach_toSignal(a_callback);
}
void InputManager::detach_FromMouseState(MouseState                a_mouseState,
                                         const MouseStateCallback& a_callback)
{
    m_mouseStateSignals[a_mouseState].detach_fromSignal(a_callback);
}
void InputManager::detach_FromMouseState(
    MouseState a_mouseState, const MouseStateSignal::mCallbackHandle& a_handle)
{
    m_mouseStateSignals[a_mouseState].detach_fromSignal(a_handle);
}
};  // namespace input
};  // namespace m