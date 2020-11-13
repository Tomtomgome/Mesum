#ifndef M_KERNEL
#define M_KERNEL
#pragma once

#include <Types.hpp>
#include <Asserts.hpp>

#include <map>
#include <string>
#include <sstream>
#include <set>

namespace m
{
    struct CmdLine
    {
        inline void register_cmdParameteredArgs(const std::wstring a_key)
        {
            mBool isInserted = m_keysParameteredArgs.insert(a_key).second;
            mAssert(isInserted);
        }

        void parse_cmdLineAguments(Int argc, Char** argv);

        template <class T>
        const mBool get_parameter(const std::wstring a_key, T& a_result) const
        {
            auto found = m_keysParams.find(a_key);
            if(found == m_keysParams.end())
            {
                return false;
            }

            std::istringstream(found.second) >> a_result;
            return true;
        }

        const mBool get_arg(const std::wstring a_arg) const
        {
            return m_foundArgs.find(a_arg) != m_foundArgs.end();
        }

        std::set<std::wstring> m_keysParameteredArgs;
        std::set<std::wstring> m_foundArgs;
        std::map<std::wstring, std::wstring> m_keysParams;
    };
}
#endif //M_KERNEL