#ifndef M_RENDERTASK
#define M_RENDERTASK
#pragma once

#include <MesumCore/Kernel/Asserts.hpp>
#include <MesumGraphics/Common.hpp>

namespace m::render
{
struct Taskset;

struct Task
{
    virtual ~Task() = default;

    virtual void prepare() = 0;
    virtual void execute() const = 0;
};

struct TaskData
{
    Task* add_toTaskSet(Taskset* a_taskset);

    virtual Task* getNew_dx12Implementation(TaskData* a_data)
    {
        mNotImplemented return nullptr;
    }
    virtual Task* getNew_vulkanImplementation(TaskData* a_data)
    {
        mNotImplemented return nullptr;
    }
};

struct Taskset
{
    virtual ~Taskset() = default;

    virtual Task* add_task(TaskData* a_data) = 0;
    virtual void  clear()                    = 0;
};

inline Task* TaskData::add_toTaskSet(Taskset* a_taskset)
{
    return a_taskset->add_task(this);
}

}  // namespace m::render

#endif  // M_RENDERTASK