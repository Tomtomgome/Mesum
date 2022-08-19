#pragma once

#include "../Common/CoreCommon.hpp"
#include "Types.hpp"

#include <string>
#include <vector>

namespace m::memory
{
template <typename t_Type, typename... t_Arg>
concept constructible = requires(t_Arg... a_args)
{
    new t_Type(a_args...);
};

using mMemoryType                            = mInt;
static const mMemoryType g_defaultMemoryType = -1;

struct mMemoryStat
{
    std::string name{};
    mBool       isVerbose          = false;
    mSize       totalAllocatedSize = 0;
};

struct mMemoryStats
{
    mSize                    globalAllocationSizes;
    std::vector<mMemoryStat> typedMemoryStats;
};

extern mMemoryStats g_memStats;

void initialize_memoryTracking();
void terminate_memoryTracking();

mMemoryType create_newMemoryType(std::string const& a_name);
void        log_memoryAllocation(mMemoryType a_type, mSize a_size);
void        log_memoryDeallocation(mMemoryType a_type, mSize a_size);

mPtr allocate(m::mSize a_size);
void deallocate(mPtr& a_object);

class mObjectAllocator
{
   public:
    void init(mMemoryType const& a_memoryType) { m_memoryType = a_memoryType; }

    template <typename t_Type, typename... t_Arg>
    requires memory::constructible<t_Type, t_Arg...> t_Type* construct(
        t_Arg... a_args)
    {
#ifdef M_TRACK_MEMORY_ALLOCATIONS
        memory::log_memoryAllocation(m_memoryType, sizeof(t_Type));
#endif  // M_TRACK_MEMORY_ALLOCATIONS
        return new t_Type(a_args...);
    }

    template <typename t_Type>
    void destroy(t_Type* a_object)
    {
#ifdef M_TRACK_MEMORY_ALLOCATIONS
        memory::log_memoryDeallocation(m_memoryType, sizeof(t_Type));
#endif  // M_TRACK_MEMORY_ALLOCATIONS
        delete a_object;
    }

   private:
    mMemoryType m_memoryType;
};

};  // namespace m::memory