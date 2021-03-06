#pragma once

//Inspired from : http://www.drdobbs.com/cpp/a-lightweight-logger-for-c/240147505
#include <MesumCore/Common.hpp>
#include <Log.hpp>

/** \addtogroup Log
*	@{
*/
namespace m
{
//! The default stream logger
extern MesumCoreApi logging::Logger<m::logging::StdcoutLogPolicy> log_inst;
};

//! Macro used to get a channel ID
#define LOG_GET_ID log_inst.get_newChannelID

#ifdef M_ENABLE_LOG
//! Macro used to log simple debug messages
#define mLOG m::log_inst.print<m::logging::SeverityType::debug>
//! Macro used to log simple error messages
#define mLOG_ERR m::log_inst.print<m::logging::SeverityType::error>
//! Macro used to log simple warning messages
#define mLOG_WARN m::log_inst.print<m::logging::SeverityType::warning>

//! Macro used to log simple debug messages to a channel
#define mLOG_TO m::log_inst.print_toChannel<m::logging::SeverityType::debug>
//! Macro used to log simple error messages to a channel
#define mLOG_ERR_TO m::log_inst.print_toChannel<m::logging::SeverityType::error>
//! Macro used to log simple warning messages to a channel
#define mLOG_WARN_TO m::log_inst.print_toChannel<m::logging::SeverityType::warning>
#else

//! Macro used to log simple debug messages
#define mLOG(...)
//! Macro used to log simple error messages
#define mLOG_ERR(...)
//! Macro used to log simple warning messages
#define mLOG_WARN(...)
//! Macro used to log simple debug messages to a channel
#define mLOG_TO(...)
//! Macro used to log simple error messages to a channel
#define mLOG_ERR_TO(...)
//! Macro used to log simple warning messages to a channel
#define mLOG_WARN_TO(...)

#endif

#ifdef M_ENABLE_VERBOSE_LOG

//! Macro used to log verbose debug messages
#define mVLOG m::log_inst.print<m::logging::SeverityType::debug>
//! Macro used to log verbose error messages
#define mVLOG_ERR m::log_inst.print<m::logging::SeverityType::error>
//! Macro used to log verbose warning messages
#define mVLOG_WARN m::log_inst.print<m::logging::SeverityType::warning>
//! Macro used to log verbose debug messages to a channel
#define mVLOG_TO m::log_inst.print_toChannel<m::logging::SeverityType::debug>
//! Macro used to log verbose error messages to a channel
#define mVLOG_ERR_TO m::log_inst.print_toChannel<m::logging::SeverityType::error>
//! Macro used to log verbose warning messages to a channel
#define mVLOG_WARN_TO m::log_inst.print_toChannel<m::logging::SeverityType::warning>

#else

//! Macro used to log verbose debug messages
#define mVLOG(...)
//! Macro used to log verbose error messages
#define mVLOG_ERR(...)
//! Macro used to log verbose warning messages
#define mVLOG_WARN(...)
//! Macro used to log verbose debug messages to a channel
#define mVLOG_TO(...)
//! Macro used to log verbose error messages to a channel
#define mVLOG_ERR_TO(...)
//! Macro used to log verbose warning messages to a channel
#define mVLOG_WARN_TO(...)

#endif

#if (defined M_ENABLE_VERBOSE_LOG) || (defined M_ENABLE_LOG)
//! Macro used to set filters for the logger
#define mLOG_FILTER m::log_inst.set_filter
//! Macro used to enable channels of the logger
#define mLOG_ENABLE m::log_inst.enable_channels
//! Macro used to disable channels the logger
#define mLOG_DISABLE m::log_inst.disable_channels

#else
//! Macro used to set filters for the logger
#define mLOG_FILTER(...)
//! Macro used to enable channels of the logger
#define mLOG_ENABLE(...)
//! Macro used to disable channels the logger
#define mLOG_DISABLE(...)
#endif

/** @}*/