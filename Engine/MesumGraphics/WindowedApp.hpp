#ifndef M_WINDOWEDAPP
#define M_WINDOWEDAPP
#pragma once

#include <MesumCore/kernel/Application.hpp>
#include <MesumCore/MesumCore/Common.hpp>
#include <MesumCore/Kernel/Mains.hpp>
#include <string>
#include <vector>


namespace m
{
    namespace windows
    {
        class IWindow;
    }
    namespace application
    {
        class IWindowedApplicationBase : public ITimedLoopApplication
        {
        public:
            virtual windows::IWindow* add_newWindow(std::wstring a_name, U32 a_width, U32 a_height) = 0;
        };
    }
};

//*****************************************************************************
// Windowed entry point
//*****************************************************************************
#if defined M_WIN32
#ifndef UNICODE
#define UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <consoleapi.h>


namespace m
{
struct WindowedLaunchData
{
    HINSTANCE m_hInstance;
    m::Int    m_nCmdShow;
};

bool init_console();
LPSTR* CommandLineToArgvA(LPWSTR lpWCmdLine, INT* pNumArgs);
}  // namespace m
#define M_EXECUTE_WINDOWED_APP(AppClass)                                     \
    m::Int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine,   \
                           m::Int nCmdShow)                                  \
    {                                                                        \
        m::WindowedLaunchData data;                                          \
        m::CmdLine            cmdLine;                                       \
        m::Int                argc;                                          \
        m::ShortChar**        argv = m::CommandLineToArgvA(pCmdLine, &argc); \
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
using m::WindowedLaunchData = m::ConsoleLaunchData;
#define M_EXECUTE_WINDOWED_APP(AppClass)           \
    int main(m::Int argc, m::ShortChar** argv)     \
    {                                              \
        m::WindowedLaunchData data;                \
        m::CmdLine            cmdLine;             \
        cmdLine.parse_cmdLineAguments(argc, argv); \
        m::internal_run<AppClass>(cmdLine, &data); \
        return 0;                                  \
    }

#endif

#endif // M_WINDOWEDAPP