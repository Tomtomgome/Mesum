#ifndef M_PLATWINDOWS
#define M_PLATWINDOWS
#pragma once
#include <Kernel/Types.hpp>
#include <Application/Application.hpp>
#include <Input/Input.hpp>

#ifndef UNICODE
#define UNICODE
#endif
#include <Windows.h>

namespace m
{
    namespace platWindows
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

        class PlatformApp : public application::ITimedLoopApplication
        {
        public:
            LRESULT process_messages(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam);

            void link_inputManager(input::InputManager* a_InputManager) { m_linkedInputManager = a_InputManager; };
        protected:
            virtual void init() override;
            virtual void destroy() override;
            virtual mBool step(const Double& a_deltaTime) override;

        private:
            input::InputManager* m_linkedInputManager;

            WIN32Context    m_W32Context;
            HWND            m_hwnd;
        };
    }
};
#endif // M_PLATWINDOWS