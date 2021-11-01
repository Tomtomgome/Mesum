#include <XCBContext.hpp>

namespace m
{
namespace xcb_unix
{
const logging::mChannelID PLATFORM_APP_ID = mLog_getId();

static input::Key translateKeys(mInt a_keyCode)
{
    using namespace input;
    // Taken from GLFW Fallback function
    switch (a_keyCode)
    {
        case VK_ESCAPE: return KEY_ESCAPE;
        case VK_TAB: return KEY_TAB;
        case VK_LSHIFT: return KEY_LEFT_SHIFT;
        case VK_RSHIFT: return KEY_RIGHT_SHIFT;
        case VK_LCONTROL: return KEY_LEFT_CONTROL;
        case VK_RCONTROL: return KEY_RIGHT_CONTROL;
        case VK_MENU:
            return KEY_LEFT_ALT;
            // case VK_MODE_SWITCH: // Mapped to Alt_R on many keyboards
            // case VK_ISO_LEVEL3_SHIFT: // AltGr on at least some machines
            // case VK_META_R:
            // case VK_ALT_R:          return KEY_RIGHT_ALT;
            // case VK_LSUPER:        return KEY_LEFT_SUPER;
            // case VK_SUPER_R:        return KEY_RIGHT_SUPER;
            // case VK_MENU:           return KEY_MENU;
        case VK_NUMLOCK: return KEY_NUM_LOCK;
        case VK_CAPITAL: return KEY_CAPS_LOCK;
        case VK_PRINT: return KEY_PRINT_SCREEN;
        case VK_SCROLL: return KEY_SCROLL_LOCK;
        case VK_PAUSE: return KEY_PAUSE;
        case VK_DELETE: return KEY_DELETE;
        case VK_BACK: return KEY_BACKSPACE;
        case VK_RETURN: return KEY_ENTER;
        case VK_HOME: return KEY_HOME;
        case VK_END: return KEY_END;
        case VK_PRIOR: return KEY_PAGE_UP;
        case VK_NEXT: return KEY_PAGE_DOWN;
        case VK_INSERT: return KEY_INSERT;
        case VK_LEFT: return KEY_LEFT;
        case VK_RIGHT: return KEY_RIGHT;
        case VK_DOWN: return KEY_DOWN;
        case VK_UP: return KEY_UP;
        case VK_F1: return KEY_F1;
        case VK_F2: return KEY_F2;
        case VK_F3: return KEY_F3;
        case VK_F4: return KEY_F4;
        case VK_F5: return KEY_F5;
        case VK_F6: return KEY_F6;
        case VK_F7: return KEY_F7;
        case VK_F8: return KEY_F8;
        case VK_F9: return KEY_F9;
        case VK_F10: return KEY_F10;
        case VK_F11: return KEY_F11;
        case VK_F12: return KEY_F12;
        case VK_F13: return KEY_F13;
        case VK_F14: return KEY_F14;
        case VK_F15: return KEY_F15;
        case VK_F16: return KEY_F16;
        case VK_F17: return KEY_F17;
        case VK_F18: return KEY_F18;
        case VK_F19: return KEY_F19;
        case VK_F20: return KEY_F20;
        case VK_F21: return KEY_F21;
        case VK_F22: return KEY_F22;
        case VK_F23: return KEY_F23;
        case VK_F24:
            return KEY_F24;

            // Numeric keypad
        case VK_DIVIDE: return KEY_KP_DIVIDE;
        case VK_MULTIPLY: return KEY_KP_MULTIPLY;
        case VK_SUBTRACT: return KEY_KP_SUBTRACT;
        case VK_ADD:
            return KEY_KP_ADD;

            // These should have been detected in secondary keysym test above!
        case VK_NUMPAD0: return KEY_KP_0;
        case VK_NUMPAD1: return KEY_KP_1;
        case VK_NUMPAD2: return KEY_KP_2;
        case VK_NUMPAD3: return KEY_KP_3;
        case VK_NUMPAD4: return KEY_KP_4;
        case VK_NUMPAD5: return KEY_KP_5;
        case VK_NUMPAD6: return KEY_KP_6;
        case VK_NUMPAD7: return KEY_KP_7;
        case VK_NUMPAD8: return KEY_KP_8;
        case VK_NUMPAD9: return KEY_KP_9;
        case VK_DECIMAL:
            return KEY_KP_DECIMAL;
            // case VK_EQUAL:       return KEY_KP_EQUAL;
            // case VK_ENTER:       return KEY_KP_ENTER;

            // Last resort: Check for printable keys (should not happen if the
            // XKB extension is available). This will give a layout dependent
            // mapping (which is wrong, and we may miss some keys, especially on
            // non-US keyboards), but it's better than nothing...
        case 0x41: return KEY_A;
        case 0x42: return KEY_B;
        case 0x43: return KEY_C;
        case 0x44: return KEY_D;
        case 0x45: return KEY_E;
        case 0x46: return KEY_F;
        case 0x47: return KEY_G;
        case 0x48: return KEY_H;
        case 0x49: return KEY_I;
        case 0x4A: return KEY_J;
        case 0x4B: return KEY_K;
        case 0x4C: return KEY_L;
        case 0x4D: return KEY_M;
        case 0x4E: return KEY_N;
        case 0x4F: return KEY_O;
        case 0x50: return KEY_P;
        case 0x51: return KEY_Q;
        case 0x52: return KEY_R;
        case 0x53: return KEY_S;
        case 0x54: return KEY_T;
        case 0x55: return KEY_U;
        case 0x56: return KEY_V;
        case 0x57: return KEY_W;
        case 0x58: return KEY_X;
        case 0x59: return KEY_Y;
        case 0x5A: return KEY_Z;
        case 0x31: return KEY_1;
        case 0x32: return KEY_2;
        case 0x33: return KEY_3;
        case 0x34: return KEY_4;
        case 0x35: return KEY_5;
        case 0x36: return KEY_6;
        case 0x37: return KEY_7;
        case 0x38: return KEY_8;
        case 0x39: return KEY_9;
        case 0x30: return KEY_0;
        case VK_SPACE: return KEY_SPACE;
        default: break;
    }

    // No matching translation was found
    return KEY_UNKNOWN;
}

void XCBContext::init(HINSTANCE& a_hInstance)
{
    mLog_to(PLATFORM_APP_ID, "Initializing XcbContext");
    init_keysLuts();
}

void XCBContext::init_keysLuts()
{

}

void XCBContext::destroy()
{
    init_keysLuts();
}

} //xcb_unix
}