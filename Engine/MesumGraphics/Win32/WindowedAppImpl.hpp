#ifndef M_WINDOWEDAPPIMPL
#define M_WINDOWEDAPPIMPL
#pragma once
#include <wrl.h>

#include <MesumGraphics/WindowedApp.hpp>
#include <Win32Context.hpp>
#include <set>

namespace m::win32
{
class IWindowedApplicationImpl : public application::IWindowedApplicationBase
{
   public:
    windows::IWindow* add_newWindow(std::string a_name, U32 a_width,
                                    U32 a_height) override;

    void render() override;

   protected:
    void init() override;
    void destroy() override;
    Bool step(const Double& a_deltaTime) override;

   private:
    WIN32Context                m_W32Context;
    std::set<windows::IWindow*> m_windows;
};

}  // namespace m::win32
#endif  // M_WINDOWEDAPPIMPL