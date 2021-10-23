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

#define mNotImplemented mInterrupt

namespace m
{
extern MesumCoreApi const logging::ChannelID ASSERT_ID;

void manage_assert(Bool a_condition, Int a_lineNumber, const Char* a_file,
                   const Char* a_message, Bool a_interrupt = true);

};  // namespace m

#define WIDE2(x) L##x
#define WIDE1(x) WIDE2(x)
#define WFILE WIDE1(__FILE__)

// PLATFORM_SPECIFIC
#ifdef M_RELEASE
#define mSoftAssert(condition)
#define mAssert(condition)

#define mExpect(condition)
#else
#define mSoftAssert(condition)                      \
    m::manage_assert(condition, __LINE__, __FILE__, \
                     "Triggered soft assertion from file : ", false);
#define mAssert(condition)                          \
    m::manage_assert(condition, __LINE__, __FILE__, \
                     "Triggered soft assertion from file : ");

#define mExpect(condition)                          \
    m::manage_assert(condition, __LINE__, __FILE__, \
                     "Precondition not matched : ");
#endif

#endif  // M_ASSERTS