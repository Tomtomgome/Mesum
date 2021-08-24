#ifndef M_RENDERTASK
#define M_RENDERTASK
#pragma once

#include <MesumCore/Kernel/Asserts.hpp>
#include <MesumGraphics/Common.hpp>

#ifdef M_DX12_RENDERER
#define mIfDx12Enabled(a_something) a_something
#else
#define mIfDx12Enabled(a_something)
#endif  // M_DX12_RENDERER

#ifdef M_VULKAN_RENDERER
#define mIfVulkanEnabled(a_something) a_something
#else
#define mIfVulkanEnabled(a_something)
#endif  // M_VULKAN_RENDERER

namespace m::render
{
struct Taskset;

struct Task
{
    virtual ~Task() = default;

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