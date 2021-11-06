#ifndef M_WINDOWS
#define M_WINDOWS
#pragma once

#include <MesumCore/Kernel/Types.hpp>
#include <Renderer.hpp>
#include <string>

namespace m
{
namespace input
{
class mIInputManager;
}

namespace windows
{
class IWindow
{
   public:
    virtual ~IWindow()     = default;
    virtual void init()    = 0;
    virtual void destroy() = 0;

    virtual void link_inputManager(input::mIInputManager* a_inputManager) = 0;
    virtual render::ISurface::HdlPtr link_renderer(
        render::IRenderer* a_renderer)                   = 0;
    virtual void set_size(mUInt a_width, mUInt a_height) = 0;
    virtual void set_windowName(std::string a_name)      = 0;
    virtual void set_asMainWindow()                      = 0;
    virtual void set_asImGuiWindow()                     = 0;

    virtual void set_fullScreen(mBool a_fullscreen) = 0;
    virtual void toggle_fullScreen()                = 0;

    virtual void attach_toDestroy(
        mCallback<void> const& a_onDestroyCallback) = 0;
    virtual void attach_toResize(
        mCallback<void, mU32, mU32> const& a_onResizeCallback) = 0;
};
}  // namespace windows
}  // namespace m
#endif  // M_WINDOWS