#include "Memory.hpp"

#include "Asserts.hpp"

#include <cstdlib>

namespace m
{
mAllocatorDefault g_allocatorDefault;
mMemoryStats      g_memoryStats{{}};

mPtr mAllocatorDefault::allocate(mSize a_size)
{
#ifdef M_TRACK_MEMORY_ALLOCATIONS
    mPtr p = malloc(a_size + sizeof(mSize));
    mAssert(p != nullptr);
    g_memoryStats.defaultAllocsSize += a_size;

    *static_cast<mSize*>(p) = a_size;
    p                       = static_cast<mU8*>(p) + sizeof(mSize);
#else
    mPtr p = malloc(a_size);
#endif
    return p;
}

void mAllocatorDefault::deallocate(mPtr a_object)
{
#ifdef M_TRACK_MEMORY_ALLOCATIONS
    mPtr p = static_cast<mU8*>(a_object) - sizeof(mSize);

    // track amount
    mSize size = *static_cast<mSize*>(p);
    g_memoryStats.defaultAllocsSize -= size;
#else
    mPtr p = a_object;
#endif
    free(p);
}
}  // namespace m

void* operator new(size_t a_size)
{
    void* p = m::g_allocatorDefault.allocate(a_size);
    return p;
}

void operator delete(void* a_p)
{
    m::g_allocatorDefault.deallocate(a_p);
}