#pragma once

// Inspired from :
// http://www.drdobbs.com/cpp/a-lightweight-logger-for-c/240147505
#include "../Common/CoreCommon.hpp"
#include "Log.hpp"

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////
namespace m
{
//! The default stream logger
extern MesumCoreApi logging::mLogger<m::logging::mStdcoutLogPolicy> log_inst;
};  // namespace m

//! Macro used to get a channel ID
#define mLog_getId m::log_inst.get_newChannelID

#ifdef M_ENABLE_LOG
//! Macro used to log simple debug messages
#define mLog m::log_inst.print<m::logging::mSeverityType::debug>
//! Macro used to log simple error messages
#define mLog_error m::log_inst.print<m::logging::mSeverityType::error>
//! Macro used to log simple warning messages
#define mLog_warning m::log_inst.print<m::logging::SeverityType::warning>

//! Macro used to log simple debug messages to a channel
#define mLog_to m::log_inst.print_toChannel<m::logging::mSeverityType::debug>
//! Macro used to log simple error messages to a channel
#define mLog_errorTo \
    m::log_inst.print_toChannel<m::logging::mSeverityType::error>
//! Macro used to log simple warning messages to a channel
#define mLog_warningTo \
    m::log_inst.print_toChannel<m::logging::mSeverityType::warning>
#else

//! Macro used to log simple debug messages
#define mLog(...)
//! Macro used to log simple error messages
#define mLog_error(...)
//! Macro used to log simple warning messages
#define mLog_warning(...)
//! Macro used to log simple debug messages to a channel
#define mLog_to(...)
//! Macro used to log simple error messages to a channel
#define mLog_errorTo(...)
//! Macro used to log simple warning messages to a channel
#define mLog_warningTo(...)

#endif

#ifdef M_ENABLE_VERBOSE_LOG

//! Macro used to log verbose debug messages
#define mLog_verbose m::log_inst.print<m::logging::mSeverityType::debug>
//! Macro used to log verbose error messages
#define mLog_verboseError m::log_inst.print<m::logging::mSeverityType::error>
//! Macro used to log verbose warning messages
#define mLog_verboseWarning m::log_inst.print<m::logging::mSeverityType::warning>
//! Macro used to log verbose debug messages to a channel
#define mLog_verboseTo m::log_inst.print_toChannel<m::logging::mSeverityType::debug>
//! Macro used to log verbose error messages to a channel
#define mLog_verboseErrorTo \
    m::log_inst.print_toChannel<m::logging::mSeverityType::error>
//! Macro used to log verbose warning messages to a channel
#define mLog_VerboseWarningTo \
    m::log_inst.print_toChannel<m::logging::mSeverityType::warning>

#else

//! Macro used to log verbose debug messages
#define mLog_verbose(...)
//! Macro used to log verbose error messages
#define mLog_verboseError(...)
//! Macro used to log verbose warning messages
#define mLog_verboseWarning(...)
//! Macro used to log verbose debug messages to a channel
#define mLog_verboseTo(...)
//! Macro used to log verbose error messages to a channel
#define mLog_verboseErrorTo(...)
//! Macro used to log verbose warning messages to a channel
#define mLog_VerboseWarningTo(...)

#endif

#if (defined M_ENABLE_VERBOSE_LOG) || (defined M_ENABLE_LOG)
//! Macro used to set filters for the logger
#define mSet_logFilter m::log_inst.set_filter
//! Macro used to enable channels of the logger
#define mEnable_logChannels m::log_inst.enable_channels
//! Macro used to disable channels the logger
#define mDisable_logChannels m::log_inst.disable_channels

#else
//! Macro used to set filters for the logger
#define mSet_logFilter(...)
//! Macro used to enable channels of the logger
#define mEnable_logChannels(...)
//! Macro used to disable channels the logger
#define mDisable_logChannels(...)
#endif

///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////