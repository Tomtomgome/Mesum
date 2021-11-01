#ifndef M_Renderer
#define M_Renderer
#pragma once

#include <MesumCore/Kernel/Callbacks.hpp>
#include <MesumCore/Kernel/Types.hpp>
#include <MesumGraphics/Common.hpp>
#include <MesumGraphics/RenderTask.hpp>

namespace m::render
{
enum class RendererApi
{
    Default,  // Given by project generation
    DX12,
    Vulkan,
    _count
};

#if defined M_WIN32

struct Win32SurfaceInitData
{
    HWND m_hwnd;
    mU32  m_width;
    mU32  m_height;
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
    mU32 m_width;
    mU32 m_height;
};

#endif

class ISurface
{
   public:
    virtual ~ISurface()                                   = default;
    virtual void init_win32(Win32SurfaceInitData& a_data) = 0;
    virtual void init_x11(X11SurfaceInitData& a_data)     = 0;

    virtual render::Taskset* addNew_renderTaskset() = 0;

    virtual void render()                          = 0;
    virtual void resize(mU32 a_width, mU32 a_height) = 0;

    virtual void destroy() = 0;

    struct Handle
    {
        ISurface* surface = nullptr;
        mBool     isValid = true;
    };

    using HdlPtr = std::shared_ptr<Handle>;
};

class IRenderer
{
   public:
    virtual ~IRenderer() = default;

    virtual void init()    = 0;
    virtual void destroy() = 0;

    virtual mBool get_supportDearImGuiMultiViewports()    = 0;
    virtual void start_dearImGuiNewFrameRenderer() const = 0;

    virtual ISurface* get_newSurface() = 0;
};

}  // namespace m::render

#endif  // M_Renderer