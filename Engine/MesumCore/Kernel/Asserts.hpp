#ifndef M_ASSERTS
#define M_ASSERTS
#pragma once

#include <MesumCore/Common.hpp>
#include <Types.hpp>
#include <Logger.hpp>

#include "signal.h"

//PLATFORM_SPECIFIC
#if defined(SIGTRAP)
#define mInterrupt raise(SIGTRAP)
#else
#define mInterrupt
#endif

namespace m
{
    extern MesumCoreApi const logging::ChannelID ASSERT_ID;

    void manage_simple_assert(Bool a_condition,
        const Int a_lineNumber, const Char* a_file);

    void manage_blocking_assert(Bool a_condition,
        const Int a_lineNumber, const Char* a_file);

};

#define WIDE2(x) L##x
#define WIDE1(x) WIDE2(x)
#define WFILE WIDE1(__FILE__)

//PLATFORM_SPECIFIC
#ifdef M_RELEASE
#define mAssert(condition)
#define mHardAssert(condition)
#else
#define mAssert(condition) m::manage_simple_assert(condition, __LINE__, WFILE);
#define mHardAssert(condition) m::manage_blocking_assert(condition, __LINE__, WFILE);
#endif

#endif //M_ASSERTS