#ifndef M_PLATWINDOWS
#define M_PLATWINDOWS
#pragma once
#include <Kernel/Types.hpp>
#include <Kernel/Kernel.hpp>
#include <Application/Application.hpp>
#include <Application/Main.hpp>
#include <Input/Input.hpp>


#include <wrl.h>

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

        class PlatformApp : public application::IPlatformAppBase
        {
        public:
            LRESULT process_messages(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam);

            const CmdLine& get_cmdLine() const { return m_cmdLineArguments; }
            void link_inputManager(input::InputManager* a_InputManager) { m_linkedInputManager = a_InputManager; };
        protected:
            virtual void configure() override {}
            virtual void init() override;
            virtual void destroy() override;
            virtual mBool step(const Double& a_deltaTime) override;

        private:
            CmdLine                 m_cmdLineArguments;
            input::InputManager*    m_linkedInputManager;

            WIN32Context    m_W32Context;
            HWND            m_hwnd;

			// By default, use windowed mode.
			// Can be toggled with the Alt+Enter or F11
			mBool g_Fullscreen = false;


			U32 g_ClientWidth = 1280;
			U32 g_ClientHeight = 720;


			// Window rectangle (used to toggle fullscreen state).
			RECT g_WindowRect;
        };
    }
};
#endif // M_PLATWINDOWS