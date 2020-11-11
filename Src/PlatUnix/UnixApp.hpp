#ifndef M_PLATUNIX
#define M_PLATUNIX
#pragma once
#include <Kernel/Types.hpp>
#include <Kernel/Application.hpp>
#include <Input/Input.hpp>
#include <X11/Xlib.h>  // Every Xlib program must include this
#include <X11/keysym.h>  // Every Xlib program must include this

namespace m
{
    extern const logging::ChannelID PLAT_UNIX_ID;

    struct X11Context
    {
        void init();
        void init_keysLuts();

        void destroy();


        input::Key get_keyFromEvent(XKeyEvent& a_event);


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
        mBool  m_up    = false;
        mBool  m_down  = false;
        mBool  m_left  = false;
        Bool  m_right = false;
    };

    class UnixApp : public ITimedApplication
    {
    private:
        virtual void init() override;
        virtual void destroy() override;
        virtual mBool step(const Double& a_deltaTime) override;

    private:
        X11Context m_contextX11;
        Window m_w;
        GC m_gc;
        Float m_x = 0.0f;
        Float m_y = 0.0f;

        input::InputManager m_inputManager;
        CubeMover           m_mover;
    };

void launchTest();
};
#endif // M_PLATUNIX