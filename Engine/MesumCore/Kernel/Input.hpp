#pragma once

#include <InputCommon.hpp>
#include <Keys.hpp>
#include <array>
#include <map>

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////
namespace m::input
{
///////////////////////////////////////////////////////////////////////////////
/// \brief Enumerating modulation keys
///////////////////////////////////////////////////////////////////////////////
enum class mKeyMod
{

    none     = 0,       //!< No modulations
    shift    = 1 << 0,  //!< Either shift key
    ctrl     = 1 << 1,  //!< Either control keys
    alt      = 1 << 2,  //!< Either alt keys
    super    = 1 << 3,  //!< Platform key
    capsLock = 1 << 4,  //!< Caps lock key
    numLock  = 1 << 5   //!< Num lock key
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Enumerating mouse buttons
///////////////////////////////////////////////////////////////////////////////
enum class mMouseButton
{
    left   = 0,  //!< Left mouse button
    right  = 1,  //!< Right mouse button
    middle = 2   //!< Middle mouse button
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Enumerating the type of input types
///////////////////////////////////////////////////////////////////////////////
enum class mInputType
{
    released,  //!< A key/button is released
    pressed,   //!< A key/button is pressed
    repeated   //!< A key/button is hold and repeated by the os
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Structure representing an action executed on a mouse button
///////////////////////////////////////////////////////////////////////////////
struct mMouseAction
{
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Shortcut function to build a mouse pressed action
    ///
    /// \param a_button The button concerned by the pressed input type
    /// \param a_keyMod The modulation of the mouse action
    ///////////////////////////////////////////////////////////////////////////
    static mMouseAction mousePressed(mMouseButton a_button,
                                     mKeyMod      a_keyMod = mKeyMod::none);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Shortcut function to build a mouse released action
    ///
    /// \param a_button The button concerned by the released input type
    /// \param a_keyMod The modulation of the mouse action
    ///////////////////////////////////////////////////////////////////////////
    static mMouseAction mouseReleased(mMouseButton a_button,
                                      mKeyMod      a_keyMod = mKeyMod::none);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///////////////////////////////////////////////////////////////////////////
    mMouseAction() = default;
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Default copy constructor
    ///
    /// \param a_MouseAction The mouse action to copy from
    ///////////////////////////////////////////////////////////////////////////
    mMouseAction(const mMouseAction& a_MouseAction) = default;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Comparison operator allowing sorting
    ///
    /// \param a_l Left hand side of the operand
    /// \param a_r Right hand side of the operand
    ///////////////////////////////////////////////////////////////////////////
    friend bool operator<(const mMouseAction& a_l, const mMouseAction& a_r)
    {
        return std::tie(a_l.action, a_l.keyMod, a_l.button) <
               std::tie(a_r.action, a_r.keyMod, a_r.button);
    }

   public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief The type of input of the action
    ///////////////////////////////////////////////////////////////////////////
    mInputType action = mInputType::released;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief The modulation of the action
    ///////////////////////////////////////////////////////////////////////////
    mKeyMod keyMod = mKeyMod::none;

    ///////////////////////////////////////////////////////////////////////////
    /// \brief The button of the action
    ///////////////////////////////////////////////////////////////////////////
    mMouseButton button = mMouseButton::left;
};

struct MouseState
{
    MouseState() = default;
    MouseState(mMouseButton a_button, mKeyMod a_keyMod = mKeyMod::none)
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

    mKeyMod      m_keyMod;
    mMouseButton m_button;
};

struct KeyAction
{
    static KeyAction keyPressed(mKey a_key, mKeyMod a_keyMod = mKeyMod::none);
    static KeyAction keyReleased(mKey a_key, mKeyMod a_keyMod = mKeyMod::none);

    KeyAction()                             = default;
    KeyAction(const KeyAction& MouseAction) = default;

    friend bool operator<(const KeyAction& l, const KeyAction& r)
    {
        return std::tie(l.m_action, l.m_keyMod, l.m_key) <
               std::tie(r.m_action, r.m_keyMod,
                        r.m_key);  // keep the same order
    }

    mInputType m_action;
    mKeyMod    m_keyMod;
    mKey       m_key;
};

class InputManager
{
   public:
    void update_KeyMods(mKeyMod a_mod, mInputType a_action);

    void process_MouseEvent(mMouseButton a_button, mInputType a_action,
                            mKeyMod a_mods);
    void process_KeyEvent(mKey a_key, int a_scancode, mInputType a_action,
                          mKeyMod a_mods);
    void process_ScrollEvent(double a_xoffset, double a_yoffset);
    void process_CursorPosition(double a_xpos, double a_ypos);

    void processAndUpdate_States();

    mKeyActionSignal::mCallbackHandle attach_ToKeyEvent(
        KeyAction a_keyAction, const mKeyActionCallback& a_callback);
    mMouseActionSignal::mCallbackHandle attach_ToMouseEvent(
        mMouseAction a_mouseAction, const mMouseActionCallback& a_callback);
    mScrollSignal::mCallbackHandle attach_ToScrollEvent(
        const mScrollCallback& a_callback);
    mMouseMoveSignal::mCallbackHandle attach_ToMoveEvent(
        const mMouseMoveCallback& a_callback);

    void detach_FromKeyEvent(KeyAction                a_keyAction,
                             const mKeyActionCallback& a_callback);
    void detach_FromKeyEvent(KeyAction a_keyAction,
                             const mKeyActionSignal::mCallbackHandle& a_handle);

    void detach_FromMouseEvent(mMouseAction               a_mouseAction,
                               const mMouseActionCallback& a_callback);
    void detach_FromMouseEvent(
        mMouseAction                              a_mouseAction,
        const mMouseActionSignal::mCallbackHandle& a_handle);

    void detach_FromScrollEvent(const mScrollCallback& a_callback);
    void detach_FromScrollEvent(
        const mScrollSignal::mCallbackHandle& a_callback);

    void detach_FromMoveEvent(const mMouseMoveCallback& a_callback);
    void detach_FromMoveEvent(
        const mMouseMoveSignal::mCallbackHandle& a_callback);

    mMouseStateSignal::mCallbackHandle attach_ToMouseState(
        MouseState a_mouseState, const mMouseStateCallback& a_callback);
    void detach_FromMouseState(MouseState                a_mouseState,
                               const mMouseStateCallback& a_callback);
    void detach_FromMouseState(
        MouseState                               a_mouseState,
        const mMouseStateSignal::mCallbackHandle& a_handle);

   private:
    double m_mousePosX;
    double m_mousePosY;

    std::array<mBool, 8>                 m_previousMouseButtonClicked = {false};
    std::array<mBool, 8>                 m_mouseButtonClicked         = {false};
    std::array<mBool, mKey::keyLast + 1> m_keyPressed;

    int                                       m_previousMod = 0;
    int                                       m_currentMod  = 0;
    std::map<KeyAction, mKeyActionSignal>      m_keySignals;
    std::map<mMouseAction, mMouseActionSignal> m_mouseSignals;

    std::map<MouseState, mMouseStateSignal> m_mouseStateSignals;

    mScrollSignal   m_scrollSignal;
    mMouseMoveSignal m_moveSignal;
};
}  // namespace m::input
///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////