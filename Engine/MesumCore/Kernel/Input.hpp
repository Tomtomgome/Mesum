#ifndef M_INPUT
#define M_INPUT
#pragma once

#include <InputCommon.hpp>
#include <Keys.hpp>

#include <map>
#include <array>

namespace m
{
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
namespace input
{
enum class KeyMod
{

    NONE      = 0,
    SHIFT     = 1 << 0,
    CTRL      = 1 << 1,
    ALT       = 1 << 2,
    SUPER     = 1 << 3,
    CAPS_LOCK = 1 << 4,
    NUM_LOCK  = 1 << 5
};

enum class MouseButton
{
    LEFT   = 0,
    RIGHT  = 1,
    MIDDLE = 2
};

enum class Action
{
    RELEASED,
    PRESSED,
    REAPETED
};

struct MouseAction
{
    static MouseAction mousePressed(MouseButton a_button,
                                    KeyMod      a_keyMod = KeyMod::NONE);
    static MouseAction mouseReleased(MouseButton a_button,
                                     KeyMod      a_keyMod = KeyMod::NONE);

    MouseAction()                               = default;
    MouseAction(const MouseAction& MouseAction) = default;

    friend bool operator<(const MouseAction& l, const MouseAction& r)
    {
        return std::tie(l.m_action, l.m_keyMod, l.m_button) <
               std::tie(r.m_action, r.m_keyMod,
                        r.m_button);  // keep the same order
    }

    Action      m_action;
    KeyMod      m_keyMod;
    MouseButton m_button;
};

struct MouseState
{
    MouseState() = default;
    MouseState(MouseButton a_button, KeyMod a_keyMod = KeyMod::NONE)
        : m_keyMod(a_keyMod),
          m_button(a_button)
    {
    }
    MouseState(const MouseState& MouseAction) = default;

    friend bool operator<(const MouseState& l, const MouseState& r)
    {
        return std::tie(l.m_keyMod, l.m_button) <
               std::tie(r.m_keyMod, r.m_button);  // keep the same order
    }

    KeyMod      m_keyMod;
    MouseButton m_button;
};

struct KeyAction
{
    static KeyAction keyPressed(Key a_key, KeyMod a_keyMod = KeyMod::NONE);
    static KeyAction keyReleased(Key a_key, KeyMod a_keyMod = KeyMod::NONE);

    KeyAction()                             = default;
    KeyAction(const KeyAction& MouseAction) = default;

    friend bool operator<(const KeyAction& l, const KeyAction& r)
    {
        return std::tie(l.m_action, l.m_keyMod, l.m_key) <
               std::tie(r.m_action, r.m_keyMod,
                        r.m_key);  // keep the same order
    }

    Action m_action;
    KeyMod m_keyMod;
    Key    m_key;
};

class InputManager
{
   public:
    void update_KeyMods(KeyMod a_mod, Action a_action);

    void process_MouseEvent(MouseButton a_button, Action a_action,
                           KeyMod a_mods);
    void process_KeyEvent(Key a_key, int a_scancode, Action a_action,
                         KeyMod a_mods);
    void process_ScrollEvent(double a_xoffset, double a_yoffset);
    void process_CursorPosition(double a_xpos, double a_ypos);

    void processAndUpdate_States();

    KeyActionSignal::mCallbackHandle attach_ToKeyEvent(
        KeyAction a_keyAction, const KeyActionCallback& a_callback);
    MouseActionSignal::mCallbackHandle attach_ToMouseEvent(
        MouseAction a_mouseAction, const MouseActionCallback& a_callback);
    ScrollSignal::mCallbackHandle attach_ToScrollEvent(const ScrollCallback& a_callback);
    MouseMoveSignal::mCallbackHandle attach_ToMoveEvent(
        const MouseMoveCallback& a_callback);

    void detach_FromKeyEvent(KeyAction                a_keyAction,
                            const KeyActionCallback& a_callback);
    void detach_FromKeyEvent(KeyAction                      a_keyAction,
                            const KeyActionSignal::mCallbackHandle& a_handle);

    void detach_FromMouseEvent(MouseAction                a_mouseAction,
                              const MouseActionCallback& a_callback);
    void detach_FromMouseEvent(MouseAction                      a_mouseAction,
                              const MouseActionSignal::mCallbackHandle& a_handle);

    void detach_FromScrollEvent(const ScrollCallback& a_callback);
    void detach_FromScrollEvent(const ScrollSignal::mCallbackHandle& a_callback);

    void detach_FromMoveEvent(const MouseMoveCallback& a_callback);
    void detach_FromMoveEvent(const MouseMoveSignal::mCallbackHandle& a_callback);

    MouseStateSignal::mCallbackHandle attach_ToMouseState(
        MouseState a_mouseState, const MouseStateCallback& a_callback);
    void detach_FromMouseState(MouseState                a_mouseState,
                              const MouseStateCallback& a_callback);
    void detach_FromMouseState(MouseState                      a_mouseState,
                              const MouseStateSignal::mCallbackHandle& a_handle);

   private:
    double m_mousePosX;
    double m_mousePosY;

    std::array<Bool, 8>                 m_previousMouseButtonClicked = {false};
    std::array<Bool, 8>                 m_mouseButtonClicked         = {false};
    std::array<Bool, Key::KEY_LAST + 1> m_keyPressed;

    int                                      m_previousMod = 0;
    int                                      m_currentMod  = 0;
    std::map<KeyAction, KeyActionSignal>     m_keySignals;
    std::map<MouseAction, MouseActionSignal> m_mouseSignals;

    std::map<MouseState, MouseStateSignal> m_mouseStateSignals;

    ScrollSignal    m_scrollSignal;
    MouseMoveSignal m_moveSignal;
};
}  // namespace input
}  // namespace m
#endif //M_INPUT