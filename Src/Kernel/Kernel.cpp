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
            m_foundArgs.insert(arg);
            if(m_keysParameteredArgs.find(arg) != m_keysParameteredArgs.end())
            {
                m_keysParams[arg] = std::wstring(argv[i++]);
            }
        }
    }

}; // namespace m
