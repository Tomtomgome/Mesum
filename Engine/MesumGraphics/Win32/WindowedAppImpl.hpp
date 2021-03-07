#ifndef M_WINDOWEDAPPIMPL
#define M_WINDOWEDAPPIMPL
#pragma once
#include <wrl.h>

#include <Win32Context.hpp>
#include <MesumGraphics/WindowedApp.hpp>

namespace m
{
namespace win32
{
class IWindowedApplicationImpl : public application::IWindowedApplicationBase
{
   public:
    virtual windows::IWindow* add_newWindow(std::wstring a_name, U32 a_width,
                                  U32 a_height);

   protected:
    virtual void init() override;
    virtual void destroy() override;
    virtual Bool step(const Double& a_deltaTime) override;

   private:
    WIN32Context m_W32Context;

    std::vector<windows::IWindow*> m_windows;
};
}  // namespace win32
}  // namespace m
#endif  // M_WINDOWEDAPPIMPL