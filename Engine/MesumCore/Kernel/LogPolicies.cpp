#include <LogPolicies.hpp>

namespace m
{
namespace logging
{
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
void FileLogPolicy::open_ostream(const std::string& a_name)
{
    m_outStream->open(a_name.c_str(),
                      std::ios_base::binary | std::ios_base::out);
    if (!m_outStream->is_open())
    {
        throw std::runtime_error("LOGGER: unable to open an output stream");
    }
}

void FileLogPolicy::close_ostream()
{
    if (m_outStream)
    {
        m_outStream->close();
    }
}

void FileLogPolicy::write(const std::string& a_msg)
{
    (*m_outStream) << a_msg << std::endl;
}

FileLogPolicy::~FileLogPolicy()
{
    if (m_outStream)
    {
        close_ostream();
    }
}

//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
StdcoutLogPolicy::StdcoutLogPolicy() {}

void StdcoutLogPolicy::open_ostream(const std::string& a_name) {}

void StdcoutLogPolicy::close_ostream() {}

void StdcoutLogPolicy::write(const std::string& a_msg)
{
    std::cout << a_msg << std::endl;
}

StdcoutLogPolicy::~StdcoutLogPolicy() {}
}  // namespace logging
}  // namespace m