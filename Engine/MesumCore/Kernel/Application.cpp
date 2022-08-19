#include "Application.hpp"
#include "memory.hpp"

#include <chrono>
#include <thread>

namespace m::application
{
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mILoopApplication::launch(mCmdLine const& a_cmdLine, void* a_appData)
{
    memory::initialize_memoryTracking();
    init(a_cmdLine, a_appData);

    while (step())
    {
        // Nothing needed here
    }

    destroy();
    memory::terminate_memoryTracking();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mITimedLoopApplication::launch(mCmdLine const& a_cmdLine, void* a_appData)
{
    memory::initialize_memoryTracking();
    init(a_cmdLine, a_appData);

    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::steady_clock::duration deltaTime;
    do
    {
        auto end  = std::chrono::high_resolution_clock::now();
        deltaTime = end - start;
        if (deltaTime < m_minStepDuration)
        {
            std::this_thread::sleep_for(m_minStepDuration - deltaTime);
            deltaTime = m_minStepDuration;
        }
        start = std::chrono::high_resolution_clock::now();
    } while (step(deltaTime));

    destroy();
    memory::terminate_memoryTracking();
}
}  // namespace m::application