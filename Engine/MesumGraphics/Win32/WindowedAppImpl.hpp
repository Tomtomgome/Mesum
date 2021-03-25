#ifndef M_WINDOWEDAPPIMPL
#define M_WINDOWEDAPPIMPL
#pragma once
#include <wrl.h>

#include <Win32Context.hpp>
#include <MesumGraphics/WindowedApp.hpp>

#include <set>

namespace m
{
namespace win32
{

void ImGui_ImplMesum_NewFrame();

class IWindowedApplicationImpl : public application::IWindowedApplicationBase
{
   public:
    virtual windows::IWindow* add_newWindow(std::wstring a_name, U32 a_width,
                                            U32 a_height);
    virtual void set_processImGuiMultiViewports(Bool a_supportMultiViewPorts);
    virtual void render();

   protected:
    virtual void init() override;
    virtual void destroy() override;
    virtual Bool step(const Double& a_deltaTime) override;

   private:
    Bool                        m_supportImGuiMultiViewPorts = false;
    WIN32Context                m_W32Context;
    std::set<windows::IWindow*> m_windows;
};
}  // namespace win32
}  // namespace m
#endif  // M_WINDOWEDAPPIMPL