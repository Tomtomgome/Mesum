#include <MesumCore/includes.hpp>
#include <MesumCore/Kernel/Application.hpp>
#include <MesumCore/Kernel/Mains.hpp>
#include <MesumCore/Kernel/Logger.hpp>

class TestBasicApp : public m::application::mIBasicApplication
{
public:
    virtual void launch(const m::mCmdLine& a_cmdLine, void* a_appData)
 {
        m::mCmdLine const& cmdLine = get_cmdLine();
        mSoftAssert(false);
        if (cmdLine.get_arg("-N"))
        {
            mLog("Not Hello world !");
        }
        else
        {
            mLog("Hello world !");
        }
    }
};

class TestLoopedApp : public m::application::mILoopApplication
{
    virtual void init(const m::mCmdLine& a_cmdLine, void* a_appData)
    {
        mLog("Hello world !"); }
    virtual void    destroy() { mLog("Bye world !"); }
    virtual m::mBool step()
    {
        mLog("world !");
        if (m_MaxSteps-- > 0)
            return true;
        else
            return false;
    }

    m::mInt m_MaxSteps = 1000;
};

class TestTimedLoopedApp : public m::application::mITimedLoopApplication
{
    virtual void init(const m::mCmdLine& a_cmdLine, void* a_appData)
    {
        set_microSecondsLimit(16000);
        mLog("Hello world !");
    }
    virtual void    destroy() { mLog("Bye world !"); }
    virtual m::mBool step(
        const std::chrono::duration<long long int, std::nano>& a_deltaTime)
    {
        mLog("dt =", a_deltaTime);
        if (m_MaxSteps-- > 0)
            return true;
        else
            return false;
    }

    m::mInt m_MaxSteps = 1000;
};

mExecute_consoleApplication(TestTimedLoopedApp)
