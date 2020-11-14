#include <Kernel.hpp>
#include <Logger.hpp>

namespace m
{
    extern const logging::ChannelID ASSERT_ID = LOG_GET_ID();

    void CmdLine::parse_cmdLineAguments(Int argc, Char** argv)
    {
        for (Int i = 0; i < argc; ++i)
        {
            std::wstring arg(argv[i]);
            if (std::find(m_listArgs.begin(), m_listArgs.end(), arg) == m_listArgs.end())
            {
                m_listArgs.push_back(arg);
            }
        }
    }

}; // namespace m
