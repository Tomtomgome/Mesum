#ifndef M_Renderer
#define M_Renderer
#pragma once

#include <MesumCore/Kernel/Types.hpp>
#include <MesumCore/Kernel/Callbacks.hpp>
#include <MesumGraphics/Common.hpp>

namespace m
{
namespace render
{

enum class RendererApi
{
    Default, // Given by project generation
    DX12,
    Vulkan,
    _count
};

#if defined M_WIN32

struct Win32SurfaceInitData
{
    HWND m_hwnd;
    U32  m_width;
    U32  m_height;
};

struct X11SurfaceInitData
{
};

#elif defined M_UNIX

struct Win32SurfaceInitData
{
};

struct X11SurfaceInitData
{
};

#endif

class ISurface
{
   public:
    virtual ~ISurface()                                   = default;
    virtual void init_win32(Win32SurfaceInitData& a_data) = 0;
    virtual void init_x11(X11SurfaceInitData& a_data)     = 0;

    virtual void init_dearImGui(Callback<void>& a_callback) = 0;

    virtual void render()                          = 0;
    virtual void resize(U32 a_width, U32 a_height) = 0;

    virtual void destroy() = 0;
};

class IRenderer
{
   public:
    virtual void init()    = 0;
    virtual void destroy() = 0;

    virtual void start_dearImGuiNewFrame() = 0;

    virtual ISurface* get_newSurface() = 0;
};
}  // namespace render
}  // namespace m


#endif  // M_Renderer