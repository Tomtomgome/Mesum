#ifndef M_PLATUNIX
#define M_PLATUNIX
#pragma once
#include <Kernel/Types.hpp>
#include <Application/Application.hpp>
#include <Input/Input.hpp>
#include <X11/Xlib.h>  // Every Xlib program must include this
#include <X11/keysym.h>  // Every Xlib program must include this

namespace m
{
    namespace platUnix
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

        class PlatformApp : public application::ITimedLoopApplication
        {
        public:
            void link_inputManager(input::InputManager* a_InputManager) { m_linkedInputManager = a_InputManager; };
        protected:
            virtual void init() override;
            virtual void destroy() override;
            virtual mBool step(const Double& a_deltaTime) override;

        private:
            X11Context m_contextX11;
            Window m_w;
            GC m_gc;

            input::InputManager* m_linkedInputManager;
        };
    };
};
#endif // M_PLATUNIX