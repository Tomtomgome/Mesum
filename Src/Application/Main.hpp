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
    HINSTANCE       hInstance;
    PWSTR           pCmdLine;
    m::Int          nCmdShow;
};

bool InitConsole()
{
    //HACK
	static FILE* g_ic_file_cout_stream;
	static FILE* g_ic_file_cerr_stream;
    static FILE* g_ic_file_cin_stream;
	if (!AllocConsole()) { return false; }
	if (freopen_s(&g_ic_file_cout_stream, "CONOUT$", "w", stdout) != 0) { return false; } // For std::cout 
	if (freopen_s(&g_ic_file_cerr_stream, "CONERR$", "w", stderr) != 0) { return false; } // For std::cerr
	if (freopen_s(&g_ic_file_cin_stream, "CONIN$", "w+", stdin) != 0) { return false; } // For std::cin
	return true;
}

template<typename AppClass>
void internal_console_run(LaunchData& a_data)
{
    InitConsole();
    internal_run<AppClass>(a_data);
	FreeConsole();
}

template<typename AppClass>
void internal_run(LaunchData& a_data)
{
	AppClass app;
	app.setup(&a_data);
	app.launch();
}

#define EXECUTE_CONSOLE_APP(AppClass) m::Int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)\
{\
    LaunchData data = {hInstance, pCmdLine, nCmdShow};\
    internal_console_run<AppClass>(data);\
    return 0;\
}

#define EXECUTE_APP(AppClass) m::Int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)\
{\
    LaunchData data = {hInstance, pCmdLine, nCmdShow};\
    internal_run<AppClass>(data);\
    return 0;\
}
#elif defined M_UNIX

#else

#define EXECUTE_APP(AppClass) int main()\
{\
    mHardAssert(false)\
    return 0;\
}\

#endif

#endif //M_MAIN