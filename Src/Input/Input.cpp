#include <Input.hpp>

namespace m 
{
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
			return { Action::RELEASED, a_keyMod, a_button };
		}

		//------------------------------------------------------------
		//------------------------------------------------------------
		//------------------------------------------------------------
		KeyAction KeyAction::keyPressed(Key a_key, KeyMod a_keyMod)
		{
			return { Action::PRESSED, a_keyMod, a_key };
		}
		KeyAction KeyAction::keyReleased(Key a_key, KeyMod a_keyMod)
		{
			return { Action::RELEASED, a_keyMod, a_key };
		}

		//------------------------------------------------------------
		//------------------------------------------------------------
		//------------------------------------------------------------
		void InputManager::updateKeyMods(KeyMod a_mod, Action a_action)
		{
			if (a_action == Action::PRESSED && (m_currentMod & Int(a_mod)) != Int(a_mod))
			{
				m_currentMod = m_currentMod | Int(a_mod);
			}
			else if (a_action == Action::RELEASED && (m_currentMod & Int(a_mod)) == Int(a_mod))
			{
				m_currentMod = m_currentMod & !(Int(a_mod));
			}

		}

		void InputManager::processMouseEvent(MouseButton a_button, Action a_action, KeyMod a_mods)
		{
			m_mouseSignals[{a_action, a_mods, a_button}].call(math::DVec2({m_mousePosX, m_mousePosX}));
			m_mouseButtonClicked[Int(a_button)] = a_action == Action::PRESSED;
		}
		void InputManager::processKeyEvent(Key a_key, Int a_scancode, Action a_action, KeyMod a_mods)
		{
			m_keySignals[{a_action, a_mods, a_key}].call();

			if (a_key == Key::KEY_LEFT_CONTROL || a_key == Key::KEY_RIGHT_CONTROL)
			{
				updateKeyMods(KeyMod::CTRL, a_action);
			}
			else if (a_key == Key::KEY_LEFT_ALT || a_key == Key::KEY_RIGHT_ALT)
			{
				updateKeyMods(KeyMod::ALT, a_action);
			}
			else if (a_key == Key::KEY_LEFT_SHIFT || a_key == Key::KEY_RIGHT_SHIFT)
			{
				updateKeyMods(KeyMod::SHIFT, a_action);
			}
		}
		void InputManager::processScrollEvent(double a_xoffset, double a_yoffset)
		{
			m_scrollSignal.call(math::DVec2({ a_xoffset, a_yoffset }));
		}
		void InputManager::processCursorPosition(double a_xpos, double a_ypos)
		{
			m_moveSignal.call(math::DVec2({ a_xpos - m_mousePosX, a_ypos - m_mousePosY }));
			m_mousePosX = a_xpos;
			m_mousePosY = a_ypos;
		}


		void InputManager::processAndUpdateStates()
		{
			for (auto it = m_mouseStateSignals.begin(); it != m_mouseStateSignals.end(); ++it) {
				MouseState state = it->first;
				if ((Int(state.m_keyMod) & (m_currentMod ^ m_previousMod)) != 0
					|| m_mouseButtonClicked[int(state.m_button)] != m_previousMouseButtonClicked[Int(state.m_button)])
				{
					it->second.call(((Int(state.m_keyMod) & m_currentMod) == Int(state.m_keyMod))
						&& m_mouseButtonClicked[Int(state.m_button)]);
				}
			}

			m_previousMod = m_currentMod;
			m_previousMouseButtonClicked = m_mouseButtonClicked;
		}


		KeyActionSignal::handle InputManager::attachToKeyEvent(KeyAction a_keyAction, const KeyActionCallback& a_callback)
		{
			return m_keySignals[a_keyAction].attachToSignal(a_callback);
		}
		MouseActionSignal::handle InputManager::attachToMouseEvent(MouseAction a_mouseAction, const MouseActionCallback& a_callback)
		{
			return m_mouseSignals[a_mouseAction].attachToSignal(a_callback);
		}
		ScrollSignal::handle InputManager::attachToScrollEvent(const ScrollCallback& a_callback)
		{
			return m_scrollSignal.attachToSignal(a_callback);
		}
		MouseMoveSignal::handle InputManager::attachToMoveEvent(const MouseMoveCallback& a_callback)
		{
			return m_moveSignal.attachToSignal(a_callback);
		}



		void InputManager::detachFromKeyEvent(KeyAction a_keyAction, const KeyActionCallback& a_callback)
		{
			m_keySignals[a_keyAction].detachFromSignal(a_callback);
		}
		void InputManager::detachFromKeyEvent(KeyAction a_keyAction, const KeyActionSignal::handle& a_handle)
		{
			m_keySignals[a_keyAction].detachFromSignal(a_handle);
		}



		void InputManager::detachFromMouseEvent(MouseAction a_mouseAction, const MouseActionCallback& a_callback)
		{
			m_mouseSignals[a_mouseAction].detachFromSignal(a_callback);
		}
		void InputManager::detachFromMouseEvent(MouseAction a_mouseAction, const MouseActionSignal::handle& a_handle)
		{
			m_mouseSignals[a_mouseAction].detachFromSignal(a_handle);
		}


		void InputManager::detachFromScrollEvent(const ScrollCallback& a_callback)
		{
			m_scrollSignal.detachFromSignal(a_callback);
		}
		void InputManager::detachFromScrollEvent(const ScrollSignal::handle& a_handle)
		{
			m_scrollSignal.detachFromSignal(a_handle);
		}



		void InputManager::detachFromMoveEvent(const MouseMoveCallback& a_callback)
		{
			m_moveSignal.detachFromSignal(a_callback);
		}
		void InputManager::detachFromMoveEvent(const MouseMoveSignal::handle& a_handle)
		{
			m_moveSignal.detachFromSignal(a_handle);
		}


		MouseStateSignal::handle InputManager::attachToMouseState(MouseState a_mouseState, const MouseStateCallback& a_callback)
		{
			return m_mouseStateSignals[a_mouseState].attachToSignal(a_callback);
		}
		void InputManager::detachFromMouseState(MouseState a_mouseState, const MouseStateCallback& a_callback)
		{
			m_mouseStateSignals[a_mouseState].detachFromSignal(a_callback);
		}
		void InputManager::detachFromMouseState(MouseState a_mouseState, const MouseStateSignal::handle& a_handle)
		{
			m_mouseStateSignals[a_mouseState].detachFromSignal(a_handle);
		}
	};
};