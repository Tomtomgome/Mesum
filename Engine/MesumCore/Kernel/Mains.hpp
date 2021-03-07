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
void internal_run(void* a_data)
{
    AppClass app;
    app.setup(a_data);
    app.launch();
}
}  // namespace m

#define M_EXECUTE_CONSOLE_APP(AppClass)                   \
    int main(m::Int argc, m::ShortChar** argv)            \
    {                                                     \
        m::ConsoleLaunchData data;                        \
        data.m_cmdLine.parse_cmdLineAguments(argc, argv); \
        m::internal_run<AppClass>(&data);                 \
        return 0;                                         \
    }

#endif //M_MAIN