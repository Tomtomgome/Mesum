#pragma once

#include "../Common/CoreCommon.hpp"
#include "Types.hpp"

#include <string>
#include <vector>

namespace m
{
struct mMemoryType
{
    std::string name;
    mSize       size;
};

struct mMemoryStats
{
    // TODO : Need a proper start and stop for memory tracking
    // this will not be accurate for the allocations
    // during global variables allocations
    mSize                    defaultAllocsSize;
    std::vector<mMemoryType> stats;
};
extern mMemoryStats g_memoryStats;

class mIAllocator
{
   public:
    virtual ~mIAllocator() = default;

    virtual mPtr allocate(m::mSize a_size) = 0;
    virtual void deallocate(mPtr a_object) = 0;
};

class mAllocatorDefault : public mIAllocator
{
   public:
    virtual ~mAllocatorDefault() = default;

    mPtr allocate(m::mSize a_size) override;
    void deallocate(mPtr a_object) override;
};

extern mAllocatorDefault g_allocatorDefault;
}  // namespace m