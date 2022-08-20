#pragma once

#include "../Common/CoreCommon.hpp"
#include "Kernel.hpp"
#include "Types.hpp"
#include "memory.hpp"

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
void launch_internal(mCmdLine const& a_cmdLine, void* a_data)
{
    t_AppClass app;
    app.launch(a_cmdLine, a_data);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Run the process. Will launch the application
///
/// \tparam t_AppClass The type of application
/// \param a_cmdLine The cmdline arguments to pass to the application
/// \param a_data The application data to pass to the application
///////////////////////////////////////////////////////////////////////////////
template <typename t_AppClass>
void run_internal(mCmdLine const& a_cmdLine, void* a_data)
{
    memory::initialize_memoryTracking();
    launch_internal<t_AppClass>(a_cmdLine, a_data);
    memory::terminate_memoryTracking();
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
        m::run_internal<a_AppClass>(cmdLine, &data); \
        return 0;                                    \
    }
