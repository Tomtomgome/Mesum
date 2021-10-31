#ifndef M_MAIN
#define M_MAIN
#pragma once

#include <Kernel.hpp>
#include <MesumCore/Common.hpp>
#include <Types.hpp>

//*****************************************************************************
// Console entry point
//*****************************************************************************
namespace m
{
template <typename AppClass>
void internal_run(mCmdLine const& a_cmdLine, void* a_data)
{
    AppClass app;
    app.launch(a_cmdLine, a_data);
}
}  // namespace m

#define M_EXECUTE_CONSOLE_APP(AppClass)            \
    int main(m::Int argc, m::Char** argv)          \
    {                                              \
        m::mBasicLaunchData data;                  \
        m::mCmdLine         cmdLine;               \
        cmdLine.parse_cmdLineAguments(argc, argv); \
        m::internal_run<AppClass>(cmdLine, &data); \
        return 0;                                  \
    }

#endif  // M_MAIN