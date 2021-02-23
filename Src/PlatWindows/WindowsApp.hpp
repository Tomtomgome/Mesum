#ifndef M_PLATWINDOWS
#define M_PLATWINDOWS
#pragma once
#include <Kernel/Types.hpp>
#include <Kernel/Kernel.hpp>
#include <Application/Application.hpp>
#include <Application/Main.hpp>
#include <Input/Input.hpp>

#include <DX12Renderer/DX12Renderer.hpp>

#include <wrl.h>

namespace m
{
    namespace platWindows
    {
        extern const logging::ChannelID PLAT_WINDOWS_ID;

        struct WIN32Context
        {
            void init(HINSTANCE& a_hInstance);
            void init_keysLuts();

            void destroy();

            // Windows
			void register_windowClass(const Char* a_className, HINSTANCE a_hInstance);
			HWND create_window(const Char* a_className, std::wstring a_windowName, U32 a_clientWidth, U32 a_clientHeight) const;

            //Keys and inputs
            input::Key get_keyFromParam(WPARAM a_wParam) const;


            //Properties
            input::Key      m_lut_keycodes[256];
            I16             m_lut_scancode[input::Key::KEY_LAST + 1];

            HINSTANCE       m_hInstance;
        };

        class PlatformWindow
        {
		public:
			//Platform specific
			LRESULT process_messages(UINT a_uMsg, WPARAM a_wParam, LPARAM a_lParam);

			void init(WIN32Context const& a_winContext);
            void render();
			void destroy();

			void link_inputManager(input::InputManager* a_inputManager) { m_linkedInputManager = a_inputManager; };
			void set_size(UInt a_width, UInt a_height) { m_clientWidth = a_width; m_clientHeight = a_height; }
			void set_windowName(std::wstring a_name) { m_windowName = a_name; }
			void set_fullScreen(mBool a_fullscreen);
			void toggle_fullScreen();
		private:
			input::InputManager* m_linkedInputManager;

			HWND            m_hwnd;

			// By default, use windowed mode.
			// Can be toggled with F11
			mBool m_fullscreen = false;

			std::wstring    m_windowName;
			U32             m_clientWidth = 1280;
			U32             m_clientHeight = 720;

			// Window rectangle (used to toggle fullscreen state).
			RECT m_windowRect;

            WIN32Context const* m_parentContext;
            dx12::DX12Window m_window;
        };

        class PlatformApp : public application::ITimedLoopApplication
        {
        public:
            //Cross platform
            const CmdLine& get_cmdLine() const { return m_cmdLineArguments; }

            PlatformWindow* add_newWindow(std::wstring a_name, U32 a_width, U32 a_height);

        protected:
            virtual void init() override;
            virtual void destroy() override;
            virtual mBool step(const Double& a_deltaTime) override;


        private:
            CmdLine                 m_cmdLineArguments;
            WIN32Context    m_W32Context;

            std::vector<PlatformWindow*> m_windows;
        };
    }
};
#endif // M_PLATWINDOWS