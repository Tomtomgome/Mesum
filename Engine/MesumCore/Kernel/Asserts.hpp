#pragma once

#include "../Common/CoreCommon.hpp"
#include "Logger.hpp"
#include "Types.hpp"
#include <csignal>

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////

// PLATFORM_SPECIFIC
#if defined(SIGTRAP)
//! Stop the program
#define mInterrupt raise(SIGTRAP)
#elif defined _MSC_VER
//! Stop the program
#define mInterrupt __debugbreak();
#else
//! Stop the program
#define mInterrupt
#endif

//! Indicates that this code path is not yet implementer but should
#define mNotImplemented mInterrupt

namespace m
{
extern MesumCoreApi const logging::mChannelID g_assertLogID;

///////////////////////////////////////////////////////////////////////////////
/// \brief Managing asserts
///
/// \param a_condition The condition to check
/// \param a_lineNumber Indicates the line of code where the assert failed
/// \param a_file Indicates the file where the assert failed
/// \param a_message The message to log in the terminal
/// \param a_interrupt If the assert blocks the execution
///////////////////////////////////////////////////////////////////////////////
void manage_assert(mBool a_condition, mInt a_lineNumber, const mChar* a_file,
                   const mChar* a_message, mBool a_interrupt = true);

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
///////////////////////////////////////////////////////////////////////////////
/// \brief A soft assert is non blocking
///
/// \param a_condition The condition to check
///////////////////////////////////////////////////////////////////////////////
#define mSoftAssert(a_condition)                      \
    m::manage_assert(a_condition, __LINE__, __FILE__, \
                     "Triggered soft assertion from file : ", false);

///////////////////////////////////////////////////////////////////////////////
/// \brief A basic assert interrupting execution
///
/// \param a_condition The condition to check
///
///////////////////////////////////////////////////////////////////////////////
#define mAssert(a_condition)                          \
    m::manage_assert(a_condition, __LINE__, __FILE__, \
                     "Triggered soft assertion from file : ");

///////////////////////////////////////////////////////////////////////////////
/// \brief Blocking assert reserved to check function preconditions
///
/// \param a_condition The condition to check
///////////////////////////////////////////////////////////////////////////////
#define mExpect(a_condition)                          \
    m::manage_assert(a_condition, __LINE__, __FILE__, \
                     "Precondition not matched : ");
#endif

///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////