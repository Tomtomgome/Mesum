#ifndef M_ASSERTS
#define M_ASSERTS
#pragma once

#include <Types.hpp>
#include <Logger/Logger.hpp>

#include "signal.h"

//PLATFORM_SPECIFIC
#if defined(SIGTRAP)
#define mInterrupt raise(SIGTRAP)
#else
#define mInterrupt
#endif

namespace m
{
    //Not working as such, find better system to store the IDS and reference them
    extern const logging::ChannelID ASSERT_ID;

    inline void manage_simple_assert(mBool a_condition,
        const Int a_lineNumber, const Char* a_file)
    {
        if(!a_condition){
            LOG_WARN_TO(ASSERT_ID, 
                "Triggered assertion from file ", a_file, 
                " at Line ", a_lineNumber);
        }
    }

    inline void manage_blocking_assert(mBool a_condition,
        const Int a_lineNumber, const Char* a_file)
    {
        if(!a_condition){
            LOG_ERR_TO(ASSERT_ID, 
                "Triggered herd assertion from file ", a_file, 
                " at Line ", a_lineNumber);
            mInterrupt;
        }
    }

};

#define WIDE2(x) L##x
#define WIDE1(x) WIDE2(x)
#define WFILE WIDE1(__FILE__)

//PLATFORM_SPECIFIC
#ifdef NDEBUG
#define mAssert(condition) 
#define mHardAssert(condition) 
#else
#define mAssert(condition) m::manage_simple_assert(condition, __LINE__, WFILE);
#define mHardAssert(condition) m::manage_blocking_assert(condition, __LINE__, WFILE);
#endif

#endif //M_ASSERTS