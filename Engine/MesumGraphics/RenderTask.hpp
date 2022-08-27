#ifndef M_RENDERTASK
#define M_RENDERTASK
#pragma once

#include <MesumCore/Kernel/Kernel.hpp>
#include <MesumCore/Kernel/Asserts.hpp>
#include <MesumGraphics/Common.hpp>

#define mDefine_getForDx12Implementation(t_TaskData, t_Dx12Implementation) \
    Task* t_TaskData::getNew_dx12Implementation(TaskData* a_data)          \
    {                                                                      \
        return new t_Dx12Implementation(static_cast<t_TaskData*>(a_data)); \
    }

#define mDefine_getForVulkanImplementation(t_TaskData, t_VulkanImplementation) \
    Task* t_TaskData::getNew_vulkanImplementation(TaskData* a_data)            \
    {                                                                          \
        return new t_VulkanImplementation(static_cast<t_TaskData*>(a_data));   \
    }

namespace m::render
{
struct Taskset;

struct Task
{
    virtual ~Task() = default;

    virtual void prepare()       = 0;
    virtual void execute() const = 0;
};

struct TaskData
{
    // TODO : Remove ponter version
    Task* add_toTaskSet(Taskset* a_taskset);
    Task& add_toTaskSet(Taskset& a_taskset);

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

    virtual void execute() = 0;
};

inline Task* TaskData::add_toTaskSet(Taskset* a_taskset)
{
    return a_taskset->add_task(this);
}

inline Task& TaskData::add_toTaskSet(Taskset& a_taskset)
{
    return unref_safe(a_taskset.add_task(this));
}

}  // namespace m::render

#endif  // M_RENDERTASK