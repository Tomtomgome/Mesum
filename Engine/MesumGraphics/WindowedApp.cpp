#include <WindowedApp.hpp>
#include <MesumCore/Kernel/Asserts.hpp>

#include <shellapi.h>
#include <corecrt_io.h>
#include <fcntl.h>
#include <iosfwd>
#include <winbase.h>


#if defined M_WIN32
namespace m
{
bool init_console()
{
    // HACK
    static FILE* g_ic_file_cout_stream;
    static FILE* g_ic_file_cerr_stream;
    static FILE* g_ic_file_cin_stream;
    if (!AllocConsole())
    {
        return false;
    }
    if (freopen_s(&g_ic_file_cout_stream, "CONOUT$", "w", stdout) != 0)
    {
        return false;
    }  // For std::cout
    if (freopen_s(&g_ic_file_cerr_stream, "CONERR$", "w", stderr) != 0)
    {
        return false;
    }  // For std::cerr
    if (freopen_s(&g_ic_file_cin_stream, "CONIN$", "w+", stdin) != 0)
    {
        return false;
    }  // For std::cin
    return true;
}

LPSTR* CommandLineToArgvA(LPWSTR lpWCmdLine, INT* pNumArgs)
{
    int     retval;
    int     numArgs;
    LPWSTR* args;
    args = CommandLineToArgvW(lpWCmdLine, &numArgs);
    if (args == NULL)
        return NULL;

    int storage = numArgs * sizeof(LPSTR);
    for (int i = 0; i < numArgs; ++i)
    {
        BOOL lpUsedDefaultChar = FALSE;
        retval = WideCharToMultiByte(CP_ACP, 0, args[i], -1, NULL, 0, NULL,
                                     &lpUsedDefaultChar);
        if (!SUCCEEDED(retval))
        {
            LocalFree(args);
            return NULL;
        }

        storage += retval;
    }

    LPSTR* result = (LPSTR*)LocalAlloc(LMEM_FIXED, storage);
    if (result == NULL)
    {
        LocalFree(args);
        return NULL;
    }

    int   bufLen = storage - numArgs * sizeof(LPSTR);
    LPSTR buffer = ((LPSTR)result) + numArgs * sizeof(LPSTR);
    for (int i = 0; i < numArgs; ++i)
    {
        mAssert(bufLen > 0);
        BOOL lpUsedDefaultChar = FALSE;
        retval = WideCharToMultiByte(CP_ACP, 0, args[i], -1, buffer, bufLen,
                                     NULL, &lpUsedDefaultChar);
        if (!SUCCEEDED(retval))
        {
            LocalFree(result);
            LocalFree(args);
            return NULL;
        }

        result[i] = buffer;
        buffer += retval;
        bufLen -= retval;
    }

    LocalFree(args);

    *pNumArgs = numArgs;
    return result;
}

}  // namespace m
#endif // M_WIN32