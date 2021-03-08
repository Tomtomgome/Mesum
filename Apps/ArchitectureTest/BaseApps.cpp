#include <MesumCore/includes.hpp>
#include <MesumCore/Kernel/Application.hpp>
#include <MesumCore/Kernel/Mains.hpp>
#include <MesumCore/Kernel/Logger.hpp>


const m::logging::ChannelID CUBEAPP_ID = mLOG_GET_ID();

class TestBasicApp : public m::application::IBasicApplication
{
public:
    virtual void launch()
    {
        m::CmdLine const& cmdLine = ((m::ConsoleLaunchData*)m_appData)->m_cmdLine;
        mAssert(false);
        if (cmdLine.get_arg("-N"))
        {
            mLOG("Not Hello world !");
        }
        else
        {
            mLOG("Hello world !");
        }
    }
};

class TestLoopedApp : public m::application::ILoopApplication
{
    virtual void    init() { mLOG("Hello world !"); }
    virtual void    destroy() { mLOG("Bye world !"); }
    virtual m::Bool step()
    {
        mLOG("world !");
        if (m_MaxSteps-- > 0)
            return true;
        else
            return false;
    }

    m::Int m_MaxSteps = 1000;
};

class TestTimedLoopedApp : public m::application::ITimedLoopApplication
{
    virtual void init()
    {
        set_microSecondsLimit(16000);
        mLOG("Hello world !");
    }
    virtual void    destroy() { mLOG("Bye world !"); }
    virtual m::Bool step(const m::Double& a_deltaTime)
    {
        mLOG_TO(CUBEAPP_ID, "dt =", a_deltaTime);
        if (m_MaxSteps-- > 0)
            return true;
        else
            return false;
    }

    m::Int m_MaxSteps = 1000;
};

M_EXECUTE_CONSOLE_APP(TestTimedLoopedApp)
