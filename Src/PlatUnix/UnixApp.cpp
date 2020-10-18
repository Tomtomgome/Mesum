#include <Logger/Logger.hpp>
#include <Kernel/Asserts.hpp>
#include <Kernel/Types.hpp>
#include <Kernel/Callbacks.hpp>
#include <Input/Input.hpp>
#include <Input/Keys.hpp>
#include <UnixApp.hpp>


#include <X11/Xlib.h>  // Every Xlib program must include this
#include <X11/keysym.h>  // Every Xlib program must include this
#include <unistd.h>

#include <chrono>
#include <thread>

#define NIL (0)  // A name for the void pointer

namespace m
{
    logging::ChannelID PLAT_UNIX_ID = LOG_GET_ID();

    static input::Key translateKeySyms(const KeySym* keysyms, int width)
    {
        using namespace input;
        //Taken from GLFW Fallback function
        switch (keysyms[0])
        {
            case XK_Escape:         return KEY_ESCAPE;
            case XK_Tab:            return KEY_TAB;
            case XK_Shift_L:        return KEY_LEFT_SHIFT;
            case XK_Shift_R:        return KEY_RIGHT_SHIFT;
            case XK_Control_L:      return KEY_LEFT_CONTROL;
            case XK_Control_R:      return KEY_RIGHT_CONTROL;
            case XK_Meta_L:
            case XK_Alt_L:          return KEY_LEFT_ALT;
            case XK_Mode_switch: // Mapped to Alt_R on many keyboards
            case XK_ISO_Level3_Shift: // AltGr on at least some machines
            case XK_Meta_R:
            case XK_Alt_R:          return KEY_RIGHT_ALT;
            case XK_Super_L:        return KEY_LEFT_SUPER;
            case XK_Super_R:        return KEY_RIGHT_SUPER;
            case XK_Menu:           return KEY_MENU;
            case XK_Num_Lock:       return KEY_NUM_LOCK;
            case XK_Caps_Lock:      return KEY_CAPS_LOCK;
            case XK_Print:          return KEY_PRINT_SCREEN;
            case XK_Scroll_Lock:    return KEY_SCROLL_LOCK;
            case XK_Pause:          return KEY_PAUSE;
            case XK_Delete:         return KEY_DELETE;
            case XK_BackSpace:      return KEY_BACKSPACE;
            case XK_Return:         return KEY_ENTER;
            case XK_Home:           return KEY_HOME;
            case XK_End:            return KEY_END;
            case XK_Page_Up:        return KEY_PAGE_UP;
            case XK_Page_Down:      return KEY_PAGE_DOWN;
            case XK_Insert:         return KEY_INSERT;
            case XK_Left:           return KEY_LEFT;
            case XK_Right:          return KEY_RIGHT;
            case XK_Down:           return KEY_DOWN;
            case XK_Up:             return KEY_UP;
            case XK_F1:             return KEY_F1;
            case XK_F2:             return KEY_F2;
            case XK_F3:             return KEY_F3;
            case XK_F4:             return KEY_F4;
            case XK_F5:             return KEY_F5;
            case XK_F6:             return KEY_F6;
            case XK_F7:             return KEY_F7;
            case XK_F8:             return KEY_F8;
            case XK_F9:             return KEY_F9;
            case XK_F10:            return KEY_F10;
            case XK_F11:            return KEY_F11;
            case XK_F12:            return KEY_F12;
            case XK_F13:            return KEY_F13;
            case XK_F14:            return KEY_F14;
            case XK_F15:            return KEY_F15;
            case XK_F16:            return KEY_F16;
            case XK_F17:            return KEY_F17;
            case XK_F18:            return KEY_F18;
            case XK_F19:            return KEY_F19;
            case XK_F20:            return KEY_F20;
            case XK_F21:            return KEY_F21;
            case XK_F22:            return KEY_F22;
            case XK_F23:            return KEY_F23;
            case XK_F24:            return KEY_F24;
            case XK_F25:            return KEY_F25;

            // Numeric keypad
            case XK_KP_Divide:      return KEY_KP_DIVIDE;
            case XK_KP_Multiply:    return KEY_KP_MULTIPLY;
            case XK_KP_Subtract:    return KEY_KP_SUBTRACT;
            case XK_KP_Add:         return KEY_KP_ADD;

            // These should have been detected in secondary keysym test above!
            case XK_KP_Insert:      return KEY_KP_0;
            case XK_KP_End:         return KEY_KP_1;
            case XK_KP_Down:        return KEY_KP_2;
            case XK_KP_Page_Down:   return KEY_KP_3;
            case XK_KP_Left:        return KEY_KP_4;
            case XK_KP_Right:       return KEY_KP_6;
            case XK_KP_Home:        return KEY_KP_7;
            case XK_KP_Up:          return KEY_KP_8;
            case XK_KP_Page_Up:     return KEY_KP_9;
            case XK_KP_Delete:      return KEY_KP_DECIMAL;
            case XK_KP_Equal:       return KEY_KP_EQUAL;
            case XK_KP_Enter:       return KEY_KP_ENTER;

            // Last resort: Check for printable keys (should not happen if the XKB
            // extension is available). This will give a layout dependent mapping
            // (which is wrong, and we may miss some keys, especially on non-US
            // keyboards), but it's better than nothing...
            case XK_a:              return KEY_A;
            case XK_b:              return KEY_B;
            case XK_c:              return KEY_C;
            case XK_d:              return KEY_D;
            case XK_e:              return KEY_E;
            case XK_f:              return KEY_F;
            case XK_g:              return KEY_G;
            case XK_h:              return KEY_H;
            case XK_i:              return KEY_I;
            case XK_j:              return KEY_J;
            case XK_k:              return KEY_K;
            case XK_l:              return KEY_L;
            case XK_m:              return KEY_M;
            case XK_n:              return KEY_N;
            case XK_o:              return KEY_O;
            case XK_p:              return KEY_P;
            case XK_q:              return KEY_Q;
            case XK_r:              return KEY_R;
            case XK_s:              return KEY_S;
            case XK_t:              return KEY_T;
            case XK_u:              return KEY_U;
            case XK_v:              return KEY_V;
            case XK_w:              return KEY_W;
            case XK_x:              return KEY_X;
            case XK_y:              return KEY_Y;
            case XK_z:              return KEY_Z;
            case XK_1:              return KEY_1;
            case XK_2:              return KEY_2;
            case XK_3:              return KEY_3;
            case XK_4:              return KEY_4;
            case XK_5:              return KEY_5;
            case XK_6:              return KEY_6;
            case XK_7:              return KEY_7;
            case XK_8:              return KEY_8;
            case XK_9:              return KEY_9;
            case XK_0:              return KEY_0;
            case XK_space:          return KEY_SPACE;
            case XK_minus:          return KEY_MINUS;
            case XK_equal:          return KEY_EQUAL;
            case XK_bracketleft:    return KEY_LEFT_BRACKET;
            case XK_bracketright:   return KEY_RIGHT_BRACKET;
            case XK_backslash:      return KEY_BACKSLASH;
            case XK_semicolon:      return KEY_SEMICOLON;
            case XK_apostrophe:     return KEY_APOSTROPHE;
            case XK_grave:          return KEY_GRAVE_ACCENT;
            case XK_comma:          return KEY_COMMA;
            case XK_period:         return KEY_PERIOD;
            case XK_slash:          return KEY_SLASH;
            case XK_less:           return KEY_WORLD_1; // At least in some layouts...
            default:                break;
        }

        // No matching translation was found
        return KEY_UNKNOWN;
    }

    struct X11Context
    {
        void init()
        {
            m_dpy = XOpenDisplay(NIL);
            mHardAssert(m_dpy != NULL);

            init_keysLuts();
        }

        void destroy()
        {
        }

        void init_keysLuts()
        {
            memset(m_lut_keycodes, -1, sizeof(m_lut_keycodes));
            memset(m_lut_scancode, -1, sizeof(m_lut_scancode));

            Int scancodeMin;
            Int scancodeMax;
            Int scancode;
            XDisplayKeycodes(m_dpy, &scancodeMin, &scancodeMax);

            Int     width;
            KeySym* keysyms =
                XGetKeyboardMapping(m_dpy, scancodeMin,
                                    scancodeMax - scancodeMin + 1, &width);

            for (scancode = scancodeMin; scancode <= scancodeMax; scancode++)
            {
                // Translate the un-translated key codes using traditional X11
                // KeySym lookups
                if (m_lut_keycodes[scancode] < 0)
                {
                    const size_t base = (scancode - scancodeMin) * width;
                    m_lut_keycodes[scancode] =
                        translateKeySyms(&keysyms[base], width);
                }

                // Store the reverse translation for faster key name lookup
                if (m_lut_keycodes[scancode] > 0)
                    m_lut_scancode[m_lut_keycodes[scancode]] =
                        scancode;
            }

            XFree(keysyms);
        }

        input::Key get_keyFromEvent(XKeyEvent& a_event)
        {
            Int scancode = a_event.keycode;
            if (scancode < 0 || scancode > 255)
            {
                return input::Key::KEY_UNKNOWN;
            }
            return m_lut_keycodes[scancode];
        }

        Display* m_dpy;

        input::Key m_lut_keycodes[256];
        I16 m_lut_scancode[input::Key::KEY_LAST + 1];
    };

    class CubeMover
    {
    public:
        void move(Float& x, Float& y)
        {
            if (m_up)
            {
                y -= speed;
            }
            if (m_down)
            {
                y += speed;
            }
            if (m_left)
            {
                x -= speed;
            }
            if (m_right)
            {
                x += speed;
            }
        }

        void set_moveUp() { m_up = true; }
        void set_moveDown() { m_down = true; }
        void set_moveLeft() { m_left = true; }
        void set_moveRight() { m_right = true; }

        void set_notMoveUp() { m_up = false; }
        void set_notMoveDown() { m_down = false; }
        void set_notMoveLeft() { m_left = false; }
        void set_notMoveRight() { m_right = false; }

    private:
        Float speed   = 0.016 * 100;
        Bool  m_up    = false;
        Bool  m_down  = false;
        Bool  m_left  = false;
        Bool  m_right = false;
    };

    void launchTest()
    {
        // Open the display
        X11Context contextX11;
        contextX11.init();
        Display* dpy = contextX11.m_dpy;
        // Get some colors

        int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
        int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

        // Create the window

        Window w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 600, 400,
                                    0, blackColor, blackColor);

        // We want to get MapNotify events

        XSelectInput(dpy, w, StructureNotifyMask | KeyPressMask | KeyReleaseMask);

        // "Map" the window (that is, make it appear on the screen)

        XMapWindow(dpy, w);

        // Create a "Graphics Context"

        GC gc = XCreateGC(dpy, w, 0, NIL);

        // Tell the GC we draw using the white color

        XSetForeground(dpy, gc, whiteColor);

        // Wait for the MapNotify event

        for (;;)
        {
            XEvent e;
            XNextEvent(dpy, &e);
            if (e.type == MapNotify)
                break;
        }

        m::Float x            = 0.0f;
        m::Float y            = 0.0f;
        bool     windowClosed = false;

        input::InputManager inputManager;
        CubeMover mover;

        inputManager.attachToKeyEvent(input::KeyAction::keyPressed(input::KEY_UP), input::KeyActionCallback(&mover, &CubeMover::set_moveUp));
        inputManager.attachToKeyEvent(input::KeyAction::keyPressed(input::KEY_DOWN), input::KeyActionCallback(&mover, &CubeMover::set_moveDown));
        inputManager.attachToKeyEvent(input::KeyAction::keyPressed(input::KEY_LEFT), input::KeyActionCallback(&mover, &CubeMover::set_moveLeft));
        inputManager.attachToKeyEvent(input::KeyAction::keyPressed(input::KEY_RIGHT), input::KeyActionCallback(&mover, &CubeMover::set_moveRight));

        inputManager.attachToKeyEvent(input::KeyAction::keyReleased(input::KEY_UP), input::KeyActionCallback(&mover, &CubeMover::set_notMoveUp));
        inputManager.attachToKeyEvent(input::KeyAction::keyReleased(input::KEY_DOWN), input::KeyActionCallback(&mover, &CubeMover::set_notMoveDown));
        inputManager.attachToKeyEvent(input::KeyAction::keyReleased(input::KEY_LEFT), input::KeyActionCallback(&mover, &CubeMover::set_notMoveLeft));
        inputManager.attachToKeyEvent(input::KeyAction::keyReleased(input::KEY_RIGHT), input::KeyActionCallback(&mover, &CubeMover::set_notMoveRight));

        while (!windowClosed)
        {
            auto start = std::chrono::high_resolution_clock::now();

            m::Int queueLength = XQLength(dpy);
            for (m::Int i = 0; i < queueLength; ++i)
            {
                XEvent e;
                XNextEvent(dpy, &e);
                if (e.type == DestroyNotify)
                {
                    windowClosed = true;
                }

                if(e.type == KeyPress)
                {
                    input::Key k = contextX11.get_keyFromEvent(e.xkey);

                    inputManager.processKeyEvent(k, 0, input::Action::PRESSED, input::KeyMod::NONE);
                }

                if(e.type == KeyRelease)
                {
                    input::Key k = contextX11.get_keyFromEvent(e.xkey);

                    inputManager.processKeyEvent(k, 0, input::Action::RELEASED, input::KeyMod::NONE);
                }
            }
            inputManager.processAndUpdateStates();
            mover.move(x, y);

            XClearWindow(dpy, w);
            XDrawRectangle(dpy, w, gc, x, y, 5, 5);
            XFlush(dpy);

            auto      end = std::chrono::high_resolution_clock::now();
            long long timming =
                std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                    .count();
            if (timming < 16)
            {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(16 - timming));
            }
        }
    }
};