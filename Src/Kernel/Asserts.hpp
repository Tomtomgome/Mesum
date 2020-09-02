#ifndef M_ASSERTS
#define M_ASSERTS
#pragma once

#include "Types.hpp"
#include "Logger.hpp"

#include "signal.h"

//PLATFORM_SPECIFIC
#if defined(SIGTRAP)
#define mInterrupt raise(SIGTRAP)
#else
#define mInterrupt
#endif

namespace m
{
    logging::ChannelID ASSERT_ID = LOG_GET_ID();

    void manage_simple_assert(Bool a_condition,
        const Int a_lineNumber, const Char* a_file)
    {
        if(!a_condition){
            LOG_WARN_TO(ASSERT_ID, 
                "Triggered assertion from file ", a_file, 
                " at Line ", a_lineNumber);
        }
    }

    void manage_blocking_assert(Bool a_condition,
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

//PLATFORM_SPECIFIC
#ifdef NDEBUG
#define mAssert(condition) 
#define mHardAssert(condition) 
#else
#define mAssert(condition) m::manage_simple_assert(condition, __LINE__, __FILE__);
#define mHardAssert(condition) m::manage_blocking_assert(condition, __LINE__, __FILE__);
#endif

#endif //M_ASSERTS