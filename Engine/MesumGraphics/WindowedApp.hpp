#ifndef M_WINDOWEDAPP
#define M_WINDOWEDAPP
#pragma once

#include <MesumCore/Common/CoreCommon.hpp>
#include <MesumCore/Kernel/Application.hpp>
#include <MesumCore/Kernel/Mains.hpp>
#include <MesumGraphics/Common.hpp>
#include <MesumGraphics/Renderer.hpp>
#include <string>
#include <vector>

namespace m
{
namespace windows
{
class mIWindow;
}
namespace application
{
class IWindowedApplicationBase : public mITimedLoopApplication
{
   public:
    virtual windows::mIWindow* add_newWindow(std::string a_name, mU32 a_width,
                                            mU32 a_height) = 0;

    virtual void start_dearImGuiNewFrame(
        render::IRenderer const* a_renderer) const = 0;
};
}  // namespace application
};  // namespace m

//*****************************************************************************
// Windowed entry point
//*****************************************************************************
#if defined M_WIN32

namespace m
{
struct WindowedLaunchData
{
    HINSTANCE m_hInstance;
    m::mInt   m_nCmdShow;
};

bool init_console();

LPSTR* CommandLineToArgvA(LPWSTR lpWCmdLine, INT* pNumArgs);
}  // namespace m
#define M_EXECUTE_WINDOWED_APP(AppClass)                                     \
    m::mInt WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine,  \
                            m::mInt nCmdShow)                                \
    {                                                                        \
        m::WindowedLaunchData data;                                          \
        m::mCmdLine           cmdLine;                                       \
        m::mInt               argc;                                          \
        m::mChar**            argv = m::CommandLineToArgvA(pCmdLine, &argc); \
        if (argv != nullptr)                                                 \
        {                                                                    \
            cmdLine.parse_cmdLineAguments(argc, argv);                       \
            LocalFree(argv);                                                 \
        }                                                                    \
        data.m_hInstance = hInstance;                                        \
        data.m_nCmdShow  = nCmdShow;                                         \
        m::init_console();                                                   \
        m::internal_run<AppClass>(cmdLine, &data);                           \
        FreeConsole();                                                       \
        return 0;                                                            \
    }
#elif defined M_UNIX
namespace m
{
struct WindowedLaunchData
{
};
}  // namespace m
#define M_EXECUTE_WINDOWED_APP(AppClass)           \
    int main(m::mInt argc, m::mChar** argv)        \
    {                                              \
        m::WindowedLaunchData data;                \
        m::mCmdLine           cmdLine;             \
        cmdLine.parse_cmdLineAguments(argc, argv); \
        m::internal_run<AppClass>(cmdLine, &data); \
        return 0;                                  \
    }

#endif

#endif  // M_WINDOWEDAPP