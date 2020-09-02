#pragma once

//Inspired from : http://www.drdobbs.com/cpp/a-lightweight-logger-for-c/240147505

#include <Log.hpp>
#include <LogConfig.hpp>
/** \addtogroup Log
*	@{
*/

#ifdef SAVE_LOGS
//! The default file logger
static logging::logger<logging::file_log_policy> log_inst("execution.log");
#else
//! The default stream logger
static logging::logger<logging::stdcout_log_policy> log_inst("");
#endif

//! Macro used to get a channel ID
#define LOG_GET_ID log_inst.getNewChannelID

#ifdef LOGGING_LEVEL_1
//! Macro used to log simple debug messages
#define LOG log_inst.print<logging::severity_type::debug>
//! Macro used to log simple error messages
#define LOG_ERR log_inst.print<logging::severity_type::error>
//! Macro used to log simple warning messages
#define LOG_WARN log_inst.print<logging::severity_type::warning>

//! Macro used to log simple debug messages to a channel
#define LOG_TO log_inst.printToChannel<logging::severity_type::debug>
//! Macro used to log simple error messages to a channel
#define LOG_ERR_TO log_inst.printToChannel<logging::severity_type::error>
//! Macro used to log simple warning messages to a channel
#define LOG_WARN_TO log_inst.printToChannel<logging::severity_type::warning>
#else

//! Macro used to log simple debug messages
#define LOG(...)
//! Macro used to log simple error messages
#define LOG_ERR(...)
//! Macro used to log simple warning messages
#define LOG_WARN(...)
//! Macro used to log simple debug messages to a channel
#define LOG_TO(...)
//! Macro used to log simple error messages to a channel
#define LOG_ERR_TO(...)
//! Macro used to log simple warning messages to a channel
#define LOG_WARN_TO(...)

#endif

#ifdef LOGGING_LEVEL_2

//! Macro used to log verbose debug messages
#define VLOG log_inst.print<logging::severity_type::debug>
//! Macro used to log verbose error messages
#define VLOG_ERR log_inst.print<logging::severity_type::error>
//! Macro used to log verbose warning messages
#define VLOG_WARN log_inst.print<logging::severity_type::warning>
//! Macro used to log verbose debug messages to a channel
#define VLOG_TO log_inst.printToChannel<logging::severity_type::debug>
//! Macro used to log verbose error messages to a channel
#define VLOG_ERR_TO log_inst.printToChannel<logging::severity_type::error>
//! Macro used to log verbose warning messages to a channel
#define VLOG_WARN_TO log_inst.printToChannel<logging::severity_type::warning>

#else

//! Macro used to log verbose debug messages
#define VLOG(...)
//! Macro used to log verbose error messages
#define VLOG_ERR(...)
//! Macro used to log verbose warning messages
#define VLOG_WARN(...)
//! Macro used to log verbose debug messages to a channel
#define VLOG_TO(...)
//! Macro used to log verbose error messages to a channel
#define VLOG_ERR_TO(...)
//! Macro used to log verbose warning messages to a channel
#define VLOG_WARN_TO(...)

#endif

#if (defined LOGGING_LEVEL_1) || (defined LOGGING_LEVEL_2)
//! Macro used to set filters for the logger
#define LOG_FILTER log_inst.setFilter
//! Macro used to enable channels of the logger
#define LOG_ENABLE log_inst.enableChannels
//! Macro used to disable channels the logger
#define LOG_DISABLE log_inst.disableChannels

#else
//! Macro used to set filters for the logger
#define LOG_FILTER(...)
//! Macro used to enable channels of the logger
#define LOG_ENABLE(...)
//! Macro used to disable channels the logger
#define LOG_DISABLE(...)
#endif

/** @}*/