#ifndef M_MAIN
#define M_MAIN
#pragma once


#include <MesumCore/Common.hpp>
#include <Types.hpp>
#include <Kernel.hpp>

//*****************************************************************************
//Console entry point
//*****************************************************************************
namespace m
{
template <typename AppClass>
void internal_run(mCmdLine const& a_cmdLine, void* a_data)
{
    AppClass app;
    app.set_cmdLineData(a_cmdLine);
    app.setup(a_data);
    app.launch();
}
}  // namespace m

#define M_EXECUTE_CONSOLE_APP(AppClass)              \
    int main(m::Int argc, m::Char** argv)       \
    {                                                \
        m::BasicLaunchData data;                     \
        m::mCmdLine        cmdLine;                  \
        cmdLine.parse_cmdLineAguments(argc, argv);   \
        m::internal_run<AppClass>(cmdLine, &data);   \
        return 0;                                    \
    }

#endif //M_MAIN