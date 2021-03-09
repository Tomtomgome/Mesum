#ifndef M_APPLICATION
#define M_APPLICATION
#pragma once

#include <Types.hpp>
#include <Kernel.hpp>

namespace m
{
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
namespace application
{
class IBasicApplication
{
   public:
    void setup(void* a_data) { m_appData = a_data; }
    void set_cmdLineData(CmdLine const& a_cmdLine) { m_cmdLine = &a_cmdLine; }
    CmdLine const& get_cmdLine() { return *m_cmdLine; }

    virtual void launch() = 0;

   protected:
    CmdLine const* m_cmdLine;
    void*          m_appData;
};

class ITimedLoopApplication : public IBasicApplication
{
    virtual void init()                          = 0;
    virtual void destroy()                       = 0;
    virtual Bool step(const Double& a_deltaTime) = 0;

   public:
    void set_microSecondsLimit(I64 a_limit)
    {
        m_limitMicroSecondsPerFrame = a_limit;
    }
    I64 get_microSecondsLimit() { return m_limitMicroSecondsPerFrame; }

    virtual void launch() final;

   private:
    I64 m_limitMicroSecondsPerFrame;
};

class ILoopApplication : public IBasicApplication
{
    virtual void init()    = 0;
    virtual void destroy() = 0;
    virtual Bool step()    = 0;

   public:
    virtual void launch() final;
};
}  // namespace application
}  // namespace m
#endif //M_APPLICATION