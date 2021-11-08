#pragma once

#include "../Common/CoreCommon.hpp"
#include "Kernel.hpp"
#include "Types.hpp"

namespace m
{
///////////////////////////////////////////////////////////////////////////////
/// \brief Launch an application
///
/// \tparam t_AppClass The type of application
/// \param a_cmdLine The cmdline arguments to pass to the application
/// \param a_data The application data to pass to the application
///////////////////////////////////////////////////////////////////////////////
template <typename t_AppClass>
void internal_run(mCmdLine const& a_cmdLine, void* a_data)
{
    t_AppClass app;
    app.launch(a_cmdLine, a_data);
}
}  // namespace m

///////////////////////////////////////////////////////////////////////////////
/// \brief Start an application as a console application
///
/// \param a_AppClass The type of application to execute
///////////////////////////////////////////////////////////////////////////////
#define mExecute_consoleApplication(a_AppClass)      \
    int main(m::mInt argc, m::mChar** argv)          \
    {                                                \
        m::mBasicLaunchData data;                    \
        m::mCmdLine         cmdLine;                 \
        cmdLine.parse_cmdLineAguments(argc, argv);   \
        m::internal_run<a_AppClass>(cmdLine, &data); \
        return 0;                                    \
    }
