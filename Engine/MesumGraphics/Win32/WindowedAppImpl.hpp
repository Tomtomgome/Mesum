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
class IWindowedApplicationImpl : public application::IWindowedApplicationBase
{
   public:
    virtual void init_renderer(
        render::RendererApi a_renderApi = render::RendererApi::Default) override;
    virtual windows::IWindow* add_newWindow(std::string a_name, U32 a_width,
                                            U32 a_height);
    virtual void set_processImGuiMultiViewports(Bool a_supportMultiViewPorts);
    virtual void start_dearImGuiNewFrame();
    virtual void render();

   protected:
    virtual void init() override;
    virtual void destroy() override;
    virtual Bool step(const Double& a_deltaTime) override;

   private:
    Bool                        m_supportImGuiMultiViewPorts = false;
    render::IRenderer*          m_renderer = nullptr;
    WIN32Context                m_W32Context;
    std::set<windows::IWindow*> m_windows;
};
}  // namespace win32
}  // namespace m
#endif  // M_WINDOWEDAPPIMPL