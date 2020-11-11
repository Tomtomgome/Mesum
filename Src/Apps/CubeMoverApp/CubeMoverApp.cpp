#include <Kernel/Types.hpp>
#include <Application/Main.hpp>
#include <Application/Application.hpp>
#include <Logger/Logger.hpp>

#include <iostream>

class CubeMoverApp : public m::application::ITimedLoopApplication
{
    virtual void init() override
    {
        set_microSecondsLimit(16000);
    }

    virtual void destroy() override
    {

    }

    virtual m::mBool step(const m::Double& a_deltaTime) override
    {
        LOG("Bonjour !, dt = ", a_deltaTime, "ms");
        return true;
    }
};

EXECUTE_CONSOLE_APP(CubeMoverApp)
