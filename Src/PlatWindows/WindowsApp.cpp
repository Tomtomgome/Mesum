#include <Logger/Logger.hpp>
#include <Kernel/Asserts.hpp>
#include <Kernel/Types.hpp>
#include <Kernel/Callbacks.hpp>
#include <Input/Keys.hpp>
#include <WindowsApp.hpp>
#include <Application/Main.hpp>

#include <shellapi.h>
#include <stdlib.h>

namespace m
{
	namespace platWindows
	{
		extern const logging::ChannelID PLAT_WINDOWS_ID = LOG_GET_ID();

		LRESULT CALLBACK WindowProc(HWND a_hwnd, UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam)
		{
			PlatformApp* dataApp = reinterpret_cast<PlatformApp*>(GetWindowLongPtr(a_hwnd, GWLP_USERDATA));
			if (dataApp != NULL)
			{
				return dataApp->process_messages(a_uMsg, a_wParam, a_lParam);
			}
			else
			{
				return DefWindowProc(a_hwnd, a_uMsg, a_wParam, a_lParam);
			}
		}

		static input::Key translateKeys(Int a_keyCode)
		{
			using namespace input;
			//Taken from GLFW Fallback function
			switch (a_keyCode)
			{
			case VK_ESCAPE:         return KEY_ESCAPE;
			case VK_TAB:            return KEY_TAB;
			case VK_LSHIFT:        return KEY_LEFT_SHIFT;
			case VK_RSHIFT:        return KEY_RIGHT_SHIFT;
			case VK_LCONTROL:      return KEY_LEFT_CONTROL;
			case VK_RCONTROL:      return KEY_RIGHT_CONTROL;
			case VK_MENU:          return KEY_LEFT_ALT;
				//case VK_MODE_SWITCH: // Mapped to Alt_R on many keyboards
				//case VK_ISO_LEVEL3_SHIFT: // AltGr on at least some machines
				//case VK_META_R:
				//case VK_ALT_R:          return KEY_RIGHT_ALT;
				//case VK_LSUPER:        return KEY_LEFT_SUPER;
				//case VK_SUPER_R:        return KEY_RIGHT_SUPER;
				//case VK_MENU:           return KEY_MENU;
			case VK_NUMLOCK:       return KEY_NUM_LOCK;
			case VK_CAPITAL:      return KEY_CAPS_LOCK;
			case VK_PRINT:          return KEY_PRINT_SCREEN;
			case VK_SCROLL:    return KEY_SCROLL_LOCK;
			case VK_PAUSE:          return KEY_PAUSE;
			case VK_DELETE:         return KEY_DELETE;
			case VK_BACK:      return KEY_BACKSPACE;
			case VK_RETURN:         return KEY_ENTER;
			case VK_HOME:           return KEY_HOME;
			case VK_END:            return KEY_END;
			case VK_PRIOR:        return KEY_PAGE_UP;
			case VK_NEXT:      return KEY_PAGE_DOWN;
			case VK_INSERT:         return KEY_INSERT;
			case VK_LEFT:           return KEY_LEFT;
			case VK_RIGHT:          return KEY_RIGHT;
			case VK_DOWN:           return KEY_DOWN;
			case VK_UP:             return KEY_UP;
			case VK_F1:             return KEY_F1;
			case VK_F2:             return KEY_F2;
			case VK_F3:             return KEY_F3;
			case VK_F4:             return KEY_F4;
			case VK_F5:             return KEY_F5;
			case VK_F6:             return KEY_F6;
			case VK_F7:             return KEY_F7;
			case VK_F8:             return KEY_F8;
			case VK_F9:             return KEY_F9;
			case VK_F10:            return KEY_F10;
			case VK_F11:            return KEY_F11;
			case VK_F12:            return KEY_F12;
			case VK_F13:            return KEY_F13;
			case VK_F14:            return KEY_F14;
			case VK_F15:            return KEY_F15;
			case VK_F16:            return KEY_F16;
			case VK_F17:            return KEY_F17;
			case VK_F18:            return KEY_F18;
			case VK_F19:            return KEY_F19;
			case VK_F20:            return KEY_F20;
			case VK_F21:            return KEY_F21;
			case VK_F22:            return KEY_F22;
			case VK_F23:            return KEY_F23;
			case VK_F24:            return KEY_F24;

				// Numeric keypad
			case VK_DIVIDE:      return KEY_KP_DIVIDE;
			case VK_MULTIPLY:    return KEY_KP_MULTIPLY;
			case VK_SUBTRACT:    return KEY_KP_SUBTRACT;
			case VK_ADD:         return KEY_KP_ADD;

				// These should have been detected in secondary keysym test above!
			case VK_NUMPAD0:	    return KEY_KP_0;
			case VK_NUMPAD1:        return KEY_KP_1;
			case VK_NUMPAD2:		return KEY_KP_2;
			case VK_NUMPAD3:		return KEY_KP_3;
			case VK_NUMPAD4:		return KEY_KP_4;
			case VK_NUMPAD5:		return KEY_KP_5;
			case VK_NUMPAD6:		return KEY_KP_6;
			case VK_NUMPAD7:        return KEY_KP_7;
			case VK_NUMPAD8:        return KEY_KP_8;
			case VK_NUMPAD9:		return KEY_KP_9;
			case VK_DECIMAL:      return KEY_KP_DECIMAL;
				//case VK_EQUAL:       return KEY_KP_EQUAL;
				//case VK_ENTER:       return KEY_KP_ENTER;

					// Last resort: Check for printable keys (should not happen if the XKB
					// extension is available). This will give a layout dependent mapping
					// (which is wrong, and we may miss some keys, especially on non-US
					// keyboards), but it's better than nothing...
			case 0x41:              return KEY_A;
			case 0x42:              return KEY_B;
			case 0x43:              return KEY_C;
			case 0x44:              return KEY_D;
			case 0x45:              return KEY_E;
			case 0x46:              return KEY_F;
			case 0x47:              return KEY_G;
			case 0x48:              return KEY_H;
			case 0x49:              return KEY_I;
			case 0x4A:              return KEY_J;
			case 0x4B:              return KEY_K;
			case 0x4C:              return KEY_L;
			case 0x4D:              return KEY_M;
			case 0x4E:              return KEY_N;
			case 0x4F:              return KEY_O;
			case 0x50:              return KEY_P;
			case 0x51:              return KEY_Q;
			case 0x52:              return KEY_R;
			case 0x53:              return KEY_S;
			case 0x54:              return KEY_T;
			case 0x55:              return KEY_U;
			case 0x56:              return KEY_V;
			case 0x57:              return KEY_W;
			case 0x58:              return KEY_X;
			case 0x59:              return KEY_Y;
			case 0x5A:              return KEY_Z;
			case 0x31:              return KEY_1;
			case 0x32:              return KEY_2;
			case 0x33:              return KEY_3;
			case 0x34:              return KEY_4;
			case 0x35:              return KEY_5;
			case 0x36:              return KEY_6;
			case 0x37:              return KEY_7;
			case 0x38:              return KEY_8;
			case 0x39:              return KEY_9;
			case 0x30:              return KEY_0;
			case VK_SPACE:          return KEY_SPACE;
			default:                break;
			}

			// No matching translation was found
			return KEY_UNKNOWN;
		}

		void WIN32Context::init(HINSTANCE& a_hInstance)
		{
			m_hInstance = a_hInstance;
			init_keysLuts();
		}

		void WIN32Context::init_keysLuts()
		{
			memset(m_lut_keycodes, -1, sizeof(m_lut_keycodes));
			memset(m_lut_scancode, -1, sizeof(m_lut_scancode));

			for (Int scancode = 0; scancode <= 255; scancode++)
			{
				// Translate the un-translated key codes using traditional X11
				// KeySym lookups
				if (m_lut_keycodes[scancode] < 0)
				{
					m_lut_keycodes[scancode] = translateKeys(scancode);
				}

				// Store the reverse translation for faster key name lookup
				if (m_lut_keycodes[scancode] > 0)
					m_lut_scancode[m_lut_keycodes[scancode]] = scancode;
			}

		}

		void WIN32Context::destroy()
		{
			init_keysLuts();
		}

		void WIN32Context::register_windowClass(const Char* a_className, HINSTANCE a_hInstance)
		{
			WNDCLASSEXW windowClass = {};
			windowClass.cbSize = sizeof(WNDCLASSEX);
			windowClass.style = CS_HREDRAW | CS_VREDRAW;
			windowClass.lpfnWndProc = WindowProc;
			windowClass.hInstance = a_hInstance;
			windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			windowClass.lpszClassName = a_className;

			RegisterClassEx(&windowClass);
		}

		HWND WIN32Context::create_window(const Char* a_className, std::wstring a_windowName, U32 a_width, U32 a_height, HINSTANCE a_hInstance)
		{
			Int screenWidth = GetSystemMetrics(SM_CXSCREEN);
			Int screenHeight = GetSystemMetrics(SM_CYSCREEN);

			RECT windowRect = { 0, 0, a_width, a_height };
			AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

			Int windowWidth = windowRect.right - windowRect.left;
			Int windowHeight = windowRect.bottom - windowRect.top;

			Int windowPosX = std::max<Int>(0, (screenWidth - windowWidth)/2);
			Int windowPosY = std::max<Int>(0, (screenHeight - windowHeight)/2);
			// Create the window.

			HWND hwnd = CreateWindowExW(
				NULL,							// Optional window styles.
				a_className,                    // Window class
				a_windowName.c_str(),			// Window text
				WS_OVERLAPPEDWINDOW,            // Window style

				// Size and position
				windowPosX, windowPosY, windowWidth, windowHeight,

				NULL,							// Parent window
				NULL,							// Menu
				m_hInstance,					// Instance handle
				nullptr							// Additional application data
			);

			mHardAssert(hwnd != NULL);

			return hwnd;
		}


		input::Key WIN32Context::get_keyFromParam(WPARAM a_wParam)
		{
			I64 scancode = a_wParam;
			if (scancode < 0 || scancode > 255)
			{
				return input::Key::KEY_UNKNOWN;
			}
			return m_lut_keycodes[scancode];
		}

		LRESULT PlatformApp::process_messages(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam)
		{
			LRESULT result = 0;
			switch (a_uMsg)
			{
			case WM_KEYDOWN:
			{
				if (m_linkedInputManager != nullptr) {
					input::Key k = m_W32Context.get_keyFromParam(a_wParam);

					m_linkedInputManager->processKeyEvent(k, 0, input::Action::PRESSED,
						input::KeyMod::NONE);
				}
			}
			break;

			case WM_KEYUP:
			{
				if (m_linkedInputManager != nullptr) {
					input::Key k = m_W32Context.get_keyFromParam(a_wParam);

					m_linkedInputManager->processKeyEvent(k, 0, input::Action::RELEASED,
						input::KeyMod::NONE);
				}
			}
			break;

			case WM_DESTROY:
				PostQuitMessage(0);
				break;

			case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(m_hwnd, &ps);



				FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
				RECT cube = { 0, 0, 0 + 10, 0 + 10 };
				FillRect(hdc, &cube, (HBRUSH)(COLOR_BACKGROUND));

				EndPaint(m_hwnd, &ps);
			}
			break;

			default:
				result = DefWindowProc(m_hwnd, a_uMsg, a_wParam, a_lParam);
			}
			return result;
		}

		void PlatformApp::init()
		{
			LaunchData& data = *(LaunchData*)m_appData;
			//Find better way to manage arguments
			Int argc;
			Char** argv = CommandLineToArgvW(data.m_pCmdLine, &argc);
			if (argv != nullptr)
			{
				m_cmdLineArguments.parse_cmdLineAguments(argc, argv);
				LocalFree(argv);
			}

			application::IPlatformAppBase::init();

			m_W32Context.init(data.m_hInstance);

			const Char className[] = L"MainWindowClass";
			// Register the window class.
			m_W32Context.register_windowClass(className, data.m_hInstance);
			m_hwnd = m_W32Context.create_window(className, m_mainWindowName, m_clientWidth, m_clientHeight, data.m_hInstance);

			SetWindowLongPtr(m_hwnd, GWLP_USERDATA, LONG_PTR(this));

			ShowWindow(m_hwnd, data.m_nCmdShow);

		}

		void PlatformApp::destroy()
		{
			m_W32Context.destroy();
		}

		mBool PlatformApp::step(const Double& a_deltaTime)
		{
			mBool     signalKeepRunning = true;

			// Run the message loop.
			MSG msg = { };


			RedrawWindow(m_hwnd, NULL, NULL, RDW_INTERNALPAINT);



			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					signalKeepRunning = false;
				}

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			m_linkedInputManager->processAndUpdateStates();

			return signalKeepRunning;
		}
	}
};