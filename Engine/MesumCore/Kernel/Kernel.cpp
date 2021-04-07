#include <Kernel.hpp>
#include <Logger.hpp>


namespace m
{
	MesumCoreApi logging::Logger<m::logging::StdcoutLogPolicy> log_inst("");

    void CmdLine::parse_cmdLineAguments(Int argc, ShortChar** argv)
    {
        for (Int i = 0; i < argc; ++i)
        {
            std::string arg(argv[i]);
            if (std::find(m_listArgs.begin(), m_listArgs.end(), arg) == m_listArgs.end())
            {
                m_listArgs.push_back(arg);
            }
        }
    }

}; // namespace m
