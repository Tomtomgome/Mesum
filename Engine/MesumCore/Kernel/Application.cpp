#include <Application.hpp>
#include <chrono>
#include <thread>

namespace m::application
{
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mILoopApplication::launch(mCmdLine const& a_cmdLine, void* a_appData)
{
    init(a_cmdLine, a_appData);

    while (step())
    {
        // Nothing needed here
    }

    destroy();
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mITimedLoopApplication::launch(mCmdLine const& a_cmdLine, void* a_appData)
{
    init(a_cmdLine, a_appData);

    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::steady_clock::duration deltaTime;
    while (step(deltaTime))
    {
        auto end  = std::chrono::high_resolution_clock::now();
        deltaTime = end - start;
        if (deltaTime < m_minStepDuration)
        {
            std::this_thread::sleep_for(m_minStepDuration - deltaTime);
            deltaTime = m_minStepDuration;
        }
        start = std::chrono::high_resolution_clock::now();
    }

    destroy();
}
}  // namespace m::application