#ifndef M_Renderer
#define M_Renderer
#pragma once

#include <MesumCore/Kernel/Callbacks.hpp>
#include <MesumCore/Kernel/Memory.hpp>
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
    mU32 m_width;
    mU32 m_height;
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

struct IResource
{
    virtual void install()   = 0;
    virtual void uninstall() = 0;

    enum class State
    {
        Empty,
        Loading,
        Loaded,
        Installing,
        Installed
    };

    struct Handle
    {
        std::atomic<State>* m_pState         = nullptr;
        mU32                m_resourceNumber = 0;
    };
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <typename t_ResultType>
using mResult = std::pair<mBool, t_ResultType>;

class mISynchTool
{
   public:
    struct Desc
    {
        mUInt bufferCount;
    };

   public:
    virtual ~mISynchTool() = default;

    virtual void init(Desc& a_desc) = 0;
    virtual void destroy()          = 0;
};

class mIRenderTarget
{
   public:
    virtual ~mIRenderTarget() = default;
};

class mISwapchain
{
   public:
    struct Desc
    {
        mUInt bufferCount;
        mUInt width;
        mUInt height;
    };

    struct DescWin32
    {
#if defined M_WIN32
        HWND hwd;
#endif
    };

    struct Descx11
    {
#if defined M_UNIX
// TODO : linux support :(
#endif
    };

   public:
    virtual ~mISwapchain() = default;

    virtual void init_win32(Desc const&      a_desc,
                            DescWin32 const& a_descWin32)              = 0;
    virtual void init_x11(Desc const& a_config, Descx11 const& a_data) = 0;
    virtual void destroy()                                             = 0;

    virtual void resize(mU32 a_width, mU32 a_height) = 0;

    virtual Desc const& get_desc() = 0;
};

class mIApi
{
   public:
    virtual ~mIApi() = default;

    virtual void init()    = 0;
    virtual void destroy() = 0;

    virtual void start_dearImGuiNewFrameRenderer() const = 0;

    [[nodiscard]] virtual render::mISwapchain& create_swapchain() const = 0;
    virtual void destroy_swapchain(mISwapchain&) const                  = 0;

    [[nodiscard]] virtual Taskset& create_renderTaskset() const  = 0;
    virtual void destroy_renderTaskset(Taskset& a_taskset) const = 0;

    [[nodiscard]] virtual mISynchTool& create_synchTool() const    = 0;
    virtual void destroy_synchTool(mISynchTool& a_synchTool) const = 0;
};

}  // namespace m::render

#endif  // M_Renderer