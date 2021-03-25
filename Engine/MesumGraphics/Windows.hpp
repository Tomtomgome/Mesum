#ifndef M_WINDOWS
#define M_WINDOWS
#pragma once

#include <MesumCore/Kernel/Types.hpp>
#include <string>

namespace m
{
namespace input
{
class InputManager;
}
namespace windows
{
class IWindow
{
   public:
    virtual ~IWindow()     = default;
    virtual void init()    = 0;
    virtual void render()  = 0;
    virtual void destroy() = 0;

    virtual void link_inputManager(input::InputManager* a_inputManager) = 0;
    virtual void set_size(UInt a_width, UInt a_height)                  = 0;
    virtual void set_windowName(std::wstring a_name)                    = 0;
    virtual void set_asMainWindow()                                     = 0;
    virtual void set_asImGuiWindow(Bool a_supportMultiViewports)        = 0;

    virtual void set_fullScreen(Bool a_fullscreen) = 0;
    virtual void toggle_fullScreen()               = 0;
};
}  // namespace windows
}  // namespace m
#endif // M_WINDOWS