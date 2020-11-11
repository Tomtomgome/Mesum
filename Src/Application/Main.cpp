#include <Main.hpp>


bool m_init_console()
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