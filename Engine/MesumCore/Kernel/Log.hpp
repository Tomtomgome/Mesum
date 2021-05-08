#ifndef M_LOG
#define M_LOG
#pragma once
#include <string>
#include <memory>
#include <fstream>
#include <mutex>
#include <sstream>

#include <LogPolicies.hpp>
namespace m
{
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
/** \addtogroup Log
 *	@{
 */
//! Namespace grouping all logging tools.
namespace logging
{
//! Defining what a ChannelID is.
using ChannelID = uint64_t;
//! Defining what a ChannelFilter is.
using ChannelFilter = uint64_t;

//! Enumeration of the logginh severity levels.
enum SeverityType
{
    debug = 1, /*!< sets a debug severity level.*/
    error,     /*!< sets a error severity level.*/
    warning    /*!< sets a warning severity level.*/
};

//! Thread safe logger class used to write logs.
/*!
        \tparam		LogPolicy	the logging policy used by the logger.
*/
template <typename LogPolicy>
class Logger
{
   public:
    //! Construct a logger.
    /*!
            \param	a_name	the output stream name.
    */
    Logger(const std::string& a_name);

    //! Get a new channel ID
    const ChannelID get_newChannelID();

    //! Set filters on the logger.
    /*!
            \param	a_filters		The channels that will be
       allowed.
    */
    void set_filter(const ChannelFilter a_filters);

    //! Disable channels of the logger.
    /*!
            \param	a_filters		The channels to disable if
       enabled.
    */
    void disable_channels(const ChannelFilter a_filters);

    //! Enable channels of the logger.
    /*!
            \param	a_filters		The channels to enable if
       disabled.
    */
    void enable_channels(const ChannelFilter a_filters);

    //! Print things in the logs using a specified channel
    /*!
            \tparam	severity	the severity of the things to log.
            \tparam	Args		the list of things to log.
            \param	a_args		the things to log.
            \param	a_channel		the channel number to print to.
    */
    template <SeverityType severity, typename... Rest>
    void print_toChannel(uint64_t a_channel, Rest... a_args);
    //! Print things in the logs
    /*!
            \tparam	severity	the severity of the things to log.
            \tparam	Args		the list of things to log.
            \param	a_args		the things to log.
    */
    template <SeverityType severity, typename... Args>
    void print(Args... a_args);

    //! Destroy a logger.
    ~Logger();

   private:
    //!	Get the time of the day into a string format.
    std::string get_time();
    //!	Construct the header of the log line.
    std::string get_loglineHeader();

    //!	The current line to write.
    std::stringstream m_logStream;
    //!	Pointer to the logpolicy to use.
    LogPolicy* m_policy;
    //! Mutex preventing multiple writes to the output file.
    std::mutex m_mutexWrite;

    //! End of the variadic function.
    void print_impl();

    //! Variadic function implementing the writing of the output.
    /*!
            \tparam	First	type of the first thing to print.
            \tparam Rest	the rest of the types of things to print.
            \param	a_First	the first thing to print.
            \param	a_param	the rest of things to print.
    */
    template <typename First, typename... Rest>
    void print_impl(First a_parm1, Rest... a_parm);

   private:
    //! Number of the current log line.
    unsigned m_lineNumber;
    //! The filter used to filter out specific channels
    ChannelFilter m_filter = -1;
    //! The next channel id to attribute
    ChannelID m_nextChannelID = 1;
};

template <typename LogPolicy>
template <SeverityType severity, typename... Rest>
void Logger<LogPolicy>::print_toChannel(uint64_t a_channel, Rest... a_args)
{
    if (a_channel & m_filter)
    {
        print<severity>(a_args...);
    }
}

template <typename LogPolicy>
template <SeverityType severity, typename... Args>
void Logger<LogPolicy>::print(Args... a_args)
{
    m_mutexWrite.lock();
    switch (severity)
    {
        case SeverityType::debug: m_logStream << "<DEBUG> :"; break;
        case SeverityType::warning: m_logStream << "<WARNING> :"; break;
        case SeverityType::error: m_logStream << "<ERROR> :"; break;
    };
    print_impl(a_args...);
    m_mutexWrite.unlock();
}

template <typename LogPolicy>
void Logger<LogPolicy>::print_impl()
{
    m_policy->write(get_loglineHeader() + m_logStream.str());
    m_logStream.str("");
}

template <typename LogPolicy>
template <typename First, typename... Rest>
void Logger<LogPolicy>::print_impl(First a_parm1, Rest... a_parm)
{
    m_logStream << a_parm1;
    print_impl(a_parm...);
}

template <typename LogPolicy>
std::string Logger<LogPolicy>::get_time()
{
    time_t       raw_time;
    time(&raw_time);

    struct tm * timeInfo;
    #ifdef M_WIN32
    timeInfo = localtime(&raw_time);
    #else
    timeInfo = localtime(&raw_time);
    #endif
    Char buffer [80];
    strftime(buffer, 80, "%d/%m/%y - %H/%M/%S", timeInfo);

    std::string time_str(buffer);
    return time_str.substr(0, time_str.size() - 1);
}

template <typename LogPolicy>
std::string Logger<LogPolicy>::get_loglineHeader()
{
    std::stringstream header;
    header.str("");
    header.fill('0');
    header.width(7);
    header << m_lineNumber++ << " < " << get_time() << " - ";
    header.fill('0');
    header.width(7);
    header << clock() << " > ~ ";
    return header.str();
}

template <typename LogPolicy>
Logger<LogPolicy>::Logger(const std::string& a_name)
{
    m_nextChannelID = 1;
    m_filter        = -1;
    m_lineNumber    = 0;
    m_policy        = new LogPolicy;
    if (!m_policy)
    {
        throw std::runtime_error(
            "LOGGER: Unable to create the logger instance");
    }
    m_policy->open_ostream(a_name);
}

template <typename LogPolicy>
Logger<LogPolicy>::~Logger()
{
    if (m_policy)
    {
        m_policy->close_ostream();
        delete m_policy;
    }
}

template <typename LogPolicy>
const ChannelID Logger<LogPolicy>::get_newChannelID()
{
    // TODO : Fix this ID generation problem
    static ChannelID nextChannelID = 1;
    return nextChannelID <<= 1;
}

template <typename LogPolicy>
void Logger<LogPolicy>::set_filter(const ChannelFilter a_filters)
{
    m_filter = a_filters;
}

template <typename LogPolicy>
void Logger<LogPolicy>::disable_channels(const ChannelFilter a_filters)
{
    m_filter = m_filter & ~a_filters;
}

template <typename LogPolicy>
void Logger<LogPolicy>::enable_channels(const ChannelFilter a_filters)
{
    m_filter = m_filter | a_filters;
}

}  // namespace logging
}  // namespace m
#endif //M_LOG
/** @}*/