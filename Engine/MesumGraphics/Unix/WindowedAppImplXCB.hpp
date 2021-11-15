#ifndef M_WINDOWEDAPPIMPLXCB
#define M_WINDOWEDAPPIMPLXCB
#pragma once
#include <XCBContext.hpp>
#include <MesumGraphics/WindowedApp.hpp>

#include <set>

namespace m
{
namespace xcb_unix
{
class IWindowedApplicationImpl : public application::IWindowedApplicationBase
{
   public:
    virtual void init_renderer(
        render::RendererApi a_renderApi = render::RendererApi::Default) override;
    virtual windows::mIWindow* add_newWindow(std::wstring a_name, mU32 a_width,
                                            mU32 a_height);
    virtual void set_processImGuiMultiViewports(mBool a_supportMultiViewPorts);
    virtual void start_dearImGuiNewFrame();
    virtual void render();

   protected:
    virtual void init(mCmdLine const& a_cmdLine, void* a_appData) override;
    virtual void destroy() override;
    virtual mBool step(const std::chrono::duration<long long int, std::nano>&
                          a_deltaTime) override;

   private:
    mBool                       m_supportImGuiMultiViewPorts = false;
    render::IRenderer*          m_renderer = nullptr;
    XCBContext                  m_XCBContext;
    std::set<windows::mIWindow*> m_windows;
};
}  // namespace xcb_unix
}  // namespace m
#endif  // M_WINDOWEDAPPIMPLXCB