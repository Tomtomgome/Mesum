#ifndef M_KERNEL
#define M_KERNEL
#pragma once

#include <MesumCore/Common.hpp>

#include <Types.hpp>
#include <Asserts.hpp>

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>


namespace m
{
struct CmdLine
{
    void parse_cmdLineAguments(Int argc, ShortChar** argv);

    template <class T>
    const Bool get_parameter(const std::string a_key, T& a_result) const
    {
        auto found = std::find(m_listArgs.begin(), m_listArgs.end(), a_key);
        if (found == m_listArgs.end())
        {
            return false;
        }

        if (++found == m_listArgs.end())
        {
            return false;
        }

        std::basic_istringstream(*found) >> a_result;
        return true;
    }

    inline const Bool get_arg(const std::string a_arg) const
    {
        return std::find(m_listArgs.begin(), m_listArgs.end(), a_arg) !=
               m_listArgs.end();
    }

    std::vector<std::string> m_listArgs;
};

struct BasicLaunchData
{
};

}  // namespace m
#endif //M_KERNEL