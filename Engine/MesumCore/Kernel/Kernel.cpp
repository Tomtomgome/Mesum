#include <Kernel.hpp>
#include <Logger.hpp>

#ifdef M_WIN32
#include <windows.h>
#include <stringapiset.h>
#endif

namespace m
{
MesumCoreApi logging::Logger<m::logging::StdcoutLogPolicy> log_inst("");

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCmdLine::parse_cmdLineAguments(Int a_argc, Char** a_argv)
{
    for (Int i = 0; i < a_argc; ++i)
    {
        std::string arg(a_argv[i]);
        if (std::find(m_listArgs.begin(), m_listArgs.end(), arg) ==
            m_listArgs.end())
        {
            m_listArgs.push_back(arg);
        }
    }
}

#if defined M_WIN32
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
std::wstring convert_string(const std::string& a_as)
{
    // deal with trivial case of empty string
    if (a_as.empty())
        return std::wstring();

    // determine required length of new string
    size_t reqLength =
        MultiByteToWideChar(CP_UTF8, 0, a_as.c_str(), (int)a_as.length(), 0, 0);

    // construct new string of required length
    std::wstring ret(reqLength, L'\0');

    // convert old string to new string
    MultiByteToWideChar(CP_UTF8, 0, a_as.c_str(), (int)a_as.length(), &ret[0],
                        (int)ret.length());

    // return new string ( compiler should optimize this away )
    return ret;
}
#endif

};  // namespace m
