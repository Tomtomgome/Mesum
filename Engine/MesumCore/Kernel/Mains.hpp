#ifndef M_MAIN
#define M_MAIN
#pragma once


#include <MesumCore/Common.hpp>
#include <Types.hpp>
#include <Kernel.hpp>

#if defined M_WINDOWS

#ifndef UNICODE
#define UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#include <iosfwd>
#include <corecrt_io.h>
#include <fcntl.h>
#include <winbase.h>
#include <consoleapi.h>

#endif


//*****************************************************************************
//Console entry point
//*****************************************************************************
namespace m
{
template <typename AppClass>
void internal_run(ConsoleLaunchData& a_data)
{
    AppClass app;
    app.setup(&a_data);
    app.launch();
}
}  // namespace m

#define M_EXECUTE_CONSOLE_APP(AppClass)                   \
    int main(m::Int argc, m::ShortChar** argv)            \
    {                                                     \
        m::ConsoleLaunchData data;                        \
        data.m_cmdLine.parse_cmdLineAguments(argc, argv); \
        m::internal_run<AppClass>(data);                  \
        return 0;                                         \
    }


#if defined M_WINDOWED_APP

//*****************************************************************************
// Windowed entry point
//*****************************************************************************
#if defined M_WINDOWS
namespace m
{
bool init_console();
}  // namespace m
#define M_EXECUTE_WINDOWED_APP(AppClass)                                   \
    m::Int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, \
                           int nCmdShow)                                   \
    {                                                                      \
        Int    argc;                                                       \
        Char** argv = CommandLineToArgvA(pCmdLine, &argc);                 \
        if (argv != nullptr)                                               \
        {                                                                  \
            m_cmdLineArguments.parse_cmdLineAguments(argc, argv);          \
            LocalFree(argv);                                               \
        }                                                                  \
        m::ConsoleLaunchData data;                                         \
        data.m_pCmdLine.parse_cmdLineAguments(argv, argv);                 \
        data.m_hInstance = hInstance;                                      \
        data.m_nCmdShow  = nCmdShow;                                       \
        init_console();                                                    \
        m_internal_run<AppClass>(data);                                    \
        FreeConsole();                                                     \
        return 0;                                                          \
    }
#elif defined M_UNIX

#define M_EXECUTE_WINDOWED_APP(AppClass)                  \
    int main(m::Int argc, m::ShortChar** argv)            \
    {                                                     \
        m::WindowedLaunchData data;                       \
        data.m_cmdLine.parse_cmdLineAguments(argc, argv); \
        m::internal_run<AppClass>(data);                  \
        return 0;                                         \
    }

#endif
#endif //M_WINDOWED_APP
#endif //M_MAIN