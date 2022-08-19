#include "Memory.hpp"

#include "Asserts.hpp"

#include <cstdlib>

namespace m::memory
{
mMemoryStats g_memStats;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void initialize_memoryTracking()
{
    g_memStats.globalAllocationSizes = 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void terminate_memoryTracking()
{
    mBool isLeak = false;
    for (auto& memStat : g_memStats.typedMemoryStats)
    {
        if (memStat.totalAllocatedSize != 0)
        {
            mLog_error(memStat.name, " : remaining allocs : ",
                       g_memStats.globalAllocationSizes);
            isLeak = true;
        }
    }
    g_memStats.typedMemoryStats.clear();

    if (g_memStats.globalAllocationSizes != 0)
    {
        mLog_error("Global allocs remaining : ",
                   g_memStats.globalAllocationSizes);
        isLeak = true;
    }
    mAssert(!isLeak);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mMemoryType create_newMemoryType(std::string const& a_name)
{
    g_memStats.typedMemoryStats.emplace_back();
    g_memStats.typedMemoryStats.back().name               = a_name;
    g_memStats.typedMemoryStats.back().totalAllocatedSize = 0;
    return g_memStats.typedMemoryStats.size() - 1;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void log_memoryAllocation(mMemoryType a_type, mSize a_size)
{
    mAssert(a_type < g_memStats.typedMemoryStats.size());
    mMemoryStat& memoryStat = g_memStats.typedMemoryStats[a_type];
    memoryStat.totalAllocatedSize += a_size;
    if (memoryStat.isVerbose)
    {
        mLog_info(memoryStat.name, " : Alloc of size ", a_size);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void log_memoryDeallocation(mMemoryType a_type, mSize a_size)
{
    mAssert(a_type < g_memStats.typedMemoryStats.size());
    mMemoryStat& memoryStat = g_memStats.typedMemoryStats[a_type];
    memoryStat.totalAllocatedSize -= a_size;
    if (memoryStat.isVerbose)
    {
        mLog_info(memoryStat.name, " : Delete of size ", a_size);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mPtr allocate(m::mSize a_size)
{
#ifdef M_TRACK_MEMORY_ALLOCATIONS
    mPtr p = malloc(a_size + sizeof(mSize));
    mAssert(p != nullptr);
    memory::g_memStats.globalAllocationSizes += a_size;
    *static_cast<mSize*>(p) = a_size;
    p                       = static_cast<mU8*>(p) + sizeof(mSize);
#else
    mPtr p = malloc(a_size);
#endif
    return p;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void deallocate(mPtr& a_object)
{
    mAssert(a_object != nullptr);
#ifdef M_TRACK_MEMORY_ALLOCATIONS
    mPtr p = static_cast<mU8*>(a_object) - sizeof(mSize);

    // track amount
    mSize size = *static_cast<mSize*>(p);
    memory::g_memStats.globalAllocationSizes -= size;
#else
    mPtr p = a_object;
#endif
    free(p);
    a_object = nullptr;
}

};  // namespace m::memory

void* operator new(size_t a_size)
{
    void* p = m::memory::allocate(a_size);
    return p;
}

void operator delete(void* a_p)
{
    m::memory::deallocate(a_p);
}