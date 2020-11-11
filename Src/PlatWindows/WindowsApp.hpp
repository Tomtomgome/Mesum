#ifndef M_PLATWINDOWS
#define M_PLATWINDOWS
#pragma once
#include <Kernel/Types.hpp>
#include <Kernel/Application.hpp>
#include <Input/Input.hpp>

#ifndef UNICODE
#define UNICODE
#endif
#include <Windows.h>

namespace m
{
    extern const logging::ChannelID PLAT_WINDOWS_ID;

    struct WIN32Context
    {
        void init();
        void init_keysLuts();

        void destroy();

        input::Key get_keyFromParam(WPARAM a_wParam);

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
            LOG("Se deplace : ", x, ":", y);
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
        mBool  m_right = false;
    };

    class WindowsApp : public ITimedApplication
    {
    public :
		void setup(HINSTANCE a_hInstance, PWSTR a_pCmdLine, int a_nCmdShow) { m_hInstance = a_hInstance; m_pCmdLine = a_pCmdLine; m_nCmdShow = a_nCmdShow; }
        LRESULT process_messages(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam);
    
    private:
        virtual void init() override;
        virtual void destroy() override;
        virtual mBool step(const Double& a_deltaTime) override;

    private:
        HINSTANCE   m_hInstance;
        PWSTR       m_pCmdLine;
        int         m_nCmdShow;

        WIN32Context    m_W32Context;
        HWND            m_hwnd;

        Float m_x = 0.0f;
        Float m_y = 0.0f;

        input::InputManager m_inputManager;
        CubeMover           m_mover;
    };

};
#endif // M_PLATWINDOWS