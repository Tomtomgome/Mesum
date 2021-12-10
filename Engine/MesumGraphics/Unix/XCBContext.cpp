#include <XCBContext.hpp>

namespace m
{
namespace xcb_unix
{
const logging::mChannelID PLATFORM_APP_ID = mLog_getId();

static input::mKey translateKeys(mInt a_keyCode)
{
    using namespace input;
    // Taken from GLFW Fallback function
    switch (a_keyCode)
    {
        case VK_ESCAPE: return keyEscape;
        case VK_TAB: return keyTab;
        case VK_LSHIFT: return keyLeftShift;
        case VK_RSHIFT: return keyRightShift;
        case VK_LCONTROL: return keyLeftControl;
        case VK_RCONTROL: return keyRightControl;
        case VK_MENU:
            return keyLeftAlt;
            // case VK_MODE_SWITCH: // Mapped to Alt_R on many keyboards
            // case VK_ISO_LEVEL3_SHIFT: // AltGr on at least some machines
            // case VK_META_R:
            // case VK_ALT_R:          return keyRightAlt;
            // case VK_LSUPER:        return keyLeftSuper;
            // case VK_SUPER_R:        return keyRightSuper;
            // case VK_MENU:           return keyMenu;
        case VK_NUMLOCK: return keyNumLock;
        case VK_CAPITAL: return keyCapsLock;
        case VK_PRINT: return keyPrintScreen;
        case VK_SCROLL: return keyScrollLock;
        case VK_PAUSE: return keyPause;
        case VK_DELETE: return keyDelete;
        case VK_BACK: return keyBackspace;
        case VK_RETURN: return keyEnter;
        case VK_HOME: return keyHome;
        case VK_END: return keyEnd;
        case VK_PRIOR: return keyPageUp;
        case VK_NEXT: return keyPageDown;
        case VK_INSERT: return keyInsert;
        case VK_LEFT: return keyLeft;
        case VK_RIGHT: return keyRight;
        case VK_DOWN: return keyDown;
        case VK_UP: return keyUp;
        case VK_F1: return keyF1;
        case VK_F2: return keyF2;
        case VK_F3: return keyF3;
        case VK_F4: return keyF4;
        case VK_F5: return keyF5;
        case VK_F6: return keyF6;
        case VK_F7: return keyF7;
        case VK_F8: return keyF8;
        case VK_F9: return keyF9;
        case VK_F10: return keyF10;
        case VK_F11: return keyF11;
        case VK_F12: return keyF12;
        case VK_F13: return keyF13;
        case VK_F14: return keyF14;
        case VK_F15: return keyF15;
        case VK_F16: return keyF16;
        case VK_F17: return keyF17;
        case VK_F18: return keyF18;
        case VK_F19: return keyF19;
        case VK_F20: return keyF20;
        case VK_F21: return keyF21;
        case VK_F22: return keyF22;
        case VK_F23: return keyF23;
        case VK_F24:
            return keyF24;

            // Numeric keypad
        case VK_DIVIDE: return keyKPDivide;
        case VK_MULTIPLY: return keyKPMultiply;
        case VK_SUBTRACT: return keyKPSubstract;
        case VK_ADD:
            return keyKPAdd;

            // These should have been detected in secondary keysym test above!
        case VK_NUMPAD0: return keyKP0;
        case VK_NUMPAD1: return keyKP1;
        case VK_NUMPAD2: return keyKP2;
        case VK_NUMPAD3: return keyKP3;
        case VK_NUMPAD4: return keyKP4;
        case VK_NUMPAD5: return keyKP5;
        case VK_NUMPAD6: return keyKP6;
        case VK_NUMPAD7: return keyKP7;
        case VK_NUMPAD8: return keyKP8;
        case VK_NUMPAD9: return keyKP9;
        case VK_DECIMAL:
            return keyKPDecimal;
            // case VK_EQUAL:       return keyKPEqual;
            // case VK_ENTER:       return keyKPEnter;

            // Last resort: Check for printable keys (should not happen if the
            // XKB extension is available). This will give a layout dependent
            // mapping (which is wrong, and we may miss some keys, especially on
            // non-US keyboards), but it's better than nothing...
        case 0x41: return keyA;
        case 0x42: return keyB;
        case 0x43: return keyC;
        case 0x44: return keyD;
        case 0x45: return keyE;
        case 0x46: return keyF;
        case 0x47: return keyG;
        case 0x48: return keyH;
        case 0x49: return keyI;
        case 0x4A: return keyJ;
        case 0x4B: return keyK;
        case 0x4C: return keyL;
        case 0x4D: return keyM;
        case 0x4E: return keyN;
        case 0x4F: return keyO;
        case 0x50: return keyP;
        case 0x51: return keyQ;
        case 0x52: return keyR;
        case 0x53: return keyS;
        case 0x54: return keyT;
        case 0x55: return keyU;
        case 0x56: return keyV;
        case 0x57: return keyW;
        case 0x58: return keyX;
        case 0x59: return keyY;
        case 0x5A: return keyZ;
        case 0x31: return key1;
        case 0x32: return key2;
        case 0x33: return key3;
        case 0x34: return key4;
        case 0x35: return key5;
        case 0x36: return key6;
        case 0x37: return key7;
        case 0x38: return key8;
        case 0x39: return key9;
        case 0x30: return key0;
        case VK_SPACE: return keySpace;
        default: break;
    }

    // No matching translation was found
    return keyUnknown;
}

void XCBContext::init(HINSTANCE& a_hInstance)
{
    mLog_infoTo(PLATFORM_APP_ID, "Initializing XcbContext");
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