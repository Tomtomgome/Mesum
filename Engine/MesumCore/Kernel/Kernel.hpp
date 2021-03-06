#ifndef M_KERNEL
#define M_KERNEL
#pragma once

#include <MesumCore/Common.hpp>

#include <Types.hpp>
#include <Asserts.hpp>

#include <string>
#include <sstream>
#include <vector>



namespace m
{
    struct CmdLine
    {
        void parse_cmdLineAguments(Int argc, Char** argv);

        template <class T>
        const Bool get_parameter(const std::wstring a_key, T& a_result) const
        {

            auto found = std::find(m_listArgs.begin(), m_listArgs.end(), a_key);
            if(found == m_listArgs.end())
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

        inline const Bool get_arg(const std::wstring a_arg) const
        {
            return std::find(m_listArgs.begin(), m_listArgs.end(), a_arg) != m_listArgs.end();
        }

        std::vector<std::wstring> m_listArgs;
    };
}
#endif //M_KERNEL