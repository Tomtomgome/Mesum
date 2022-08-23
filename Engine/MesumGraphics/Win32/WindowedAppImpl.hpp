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
    windows::mIWindow* add_newWindow(std::string a_name, mU32 a_width,
                                     mU32  a_height,
                                     mBool a_isTransparent) override;

    void start_dearImGuiNewFrame(
        render::IRenderer const* a_renderer) const override;
    void start_dearImGuiNewFrame(render::mIApi const& a_api) const override;

    void process_messages();

   protected:
    void  init(mCmdLine const& a_cmdLine, void* a_appData) override;
    void  destroy() override;
    mBool step(std::chrono::steady_clock::duration const& a_deltaTime) override;

   private:
    mBool                        m_signalKeepRunning = true;
    Win32Context                 m_W32Context;
    std::set<windows::mIWindow*> m_windows;
};

}  // namespace m::win32
#endif  // M_WINDOWEDAPPIMPL