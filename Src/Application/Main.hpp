#ifndef M_MAIN
#define M_MAIN
#pragma once

#include <Kernel/Types.hpp>
#include <Kernel/Asserts.hpp>

#if defined M_WINDOWS

#ifndef UNICODE
#define UNICODE
#endif
#include <Windows.h>
#include <iosfwd>
#include <corecrt_io.h>
#include <fcntl.h>
#include <winbase.h>
#include <consoleapi.h>


struct LaunchData
{
    HINSTANCE       m_hInstance;
    PWSTR           m_pCmdLine;
    m::Int          m_nCmdShow;
};

bool m_init_console();

template<typename AppClass>
void m_internal_console_run(LaunchData& a_data)
{
    m_init_console();
    m_internal_run<AppClass>(a_data);
	FreeConsole();
}

template<typename AppClass>
void m_internal_run(LaunchData& a_data)
{
	AppClass app;
	app.setup(&a_data);
	app.launch();
}

#define M_EXECUTE_CONSOLE_APP(AppClass) m::Int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)\
{\
    LaunchData data = {hInstance, pCmdLine, nCmdShow};\
    m_internal_console_run<AppClass>(data);\
    return 0;\
}

#define M_EXECUTE_APP(AppClass) m::Int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)\
{\
    LaunchData data = {hInstance, pCmdLine, nCmdShow};\
    m_internal_run<AppClass>(data);\
    return 0;\
}
#elif defined M_UNIX

struct LaunchData
{
    int argc;
    char** argv;
};

template<typename AppClass>
void m_internal_run(LaunchData& a_data)
{
	AppClass app;
	app.setup(&a_data);
	app.launch();
}

#define M_EXECUTE_CONSOLE_APP(AppClass) int main(int argc, char **argv)\
{\
    LaunchData data = {argc, argv};\
    m_internal_run<AppClass>(data);\
    return 0;\
}

#define M_EXECUTE_APP(AppClass) int main(int argc, char **argv)\
{\
    LaunchData data = {argc, argv};\
    m_internal_run<AppClass>(data);\
    return 0;\
}

#else

#define EXECUTE_APP(AppClass) int main()\
{\
    mHardAssert(false)\
    return 0;\
}\

#endif

#endif //M_MAIN