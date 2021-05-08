#ifndef M_ASSERTS
#define M_ASSERTS
#pragma once

#include <Logger.hpp>
#include <MesumCore/Common.hpp>
#include <Types.hpp>

#include <csignal>

// PLATFORM_SPECIFIC
#if defined(SIGTRAP)
#define mInterrupt raise(SIGTRAP)
#elif defined _MSC_VER
#define mInterrupt __debugbreak();
#else
#define mInterrupt
#endif

namespace m
{
extern MesumCoreApi const logging::ChannelID ASSERT_ID;

void manage_simple_assert(Bool a_condition, const Int a_lineNumber,
                          const Char* a_file);

void manage_blocking_assert(Bool a_condition, const Int a_lineNumber,
                            const Char* a_file);

};  // namespace m

#define WIDE2(x) L##x
#define WIDE1(x) WIDE2(x)
#define WFILE WIDE1(__FILE__)

// PLATFORM_SPECIFIC
#ifdef M_RELEASE
#define mAssert(condition)
#define mHardAssert(condition)
#else
#define mAssert(condition) \
    m::manage_simple_assert(condition, __LINE__, __FILE__);
#define mHardAssert(condition) \
    m::manage_blocking_assert(condition, __LINE__, __FILE__);
#endif

#endif  // M_ASSERTS