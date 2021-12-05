#pragma once
#include "../Common/CoreCommon.hpp"
#include "LogPolicies.hpp"
#include "Types.hpp"
#include <fstream>
#include <limits>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// \brief Namespace grouping all logging tools.
///////////////////////////////////////////////////////////////////////////////
namespace m::logging
{
///////////////////////////////////////////////////////////////////////////////
/// \brief ChannelIDs are defined by a bit
///
/// Currently a logger can have a maximum of 64 channels
///////////////////////////////////////////////////////////////////////////////
using mChannelID = mU64;

///////////////////////////////////////////////////////////////////////////////
/// \brief Channel filters are an aggregation of channel ids
///////////////////////////////////////////////////////////////////////////////
using mChannelFilter = mU64;

///////////////////////////////////////////////////////////////////////////////
/// \brief Filter that allows all channels
///////////////////////////////////////////////////////////////////////////////
constexpr mChannelFilter g_allChannelsFilter =
    (std::numeric_limits<mU64>::max)();
///////////////////////////////////////////////////////////////////////////////
/// \brief Filter that disallows all channels
///////////////////////////////////////////////////////////////////////////////
constexpr mChannelFilter g_noChannelsFilter = 0;

///////////////////////////////////////////////////////////////////////////////
/// \brief Enumeration of the logginh severity levels
///////////////////////////////////////////////////////////////////////////////
enum mSeverityType
{
    debug = 1,  //!< sets a debug severity level
    error,      //!< sets a error severity level
    warning     //!< sets a warning severity level
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Thread safe logger class used to write logs
///
/// \tparam t_LogPolicy the logging policy used by the logger
///////////////////////////////////////////////////////////////////////////////
template <typename t_LogPolicy>
class mLogger
{
   public:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Construct a logger.
    ///
    /// \param a_name the output stream name
    ///////////////////////////////////////////////////////////////////////////
    explicit mLogger(const std::string& a_name);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Destroy a logger.
    ///////////////////////////////////////////////////////////////////////////
    ~mLogger();

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Get a new channel ID for this logger
    ///////////////////////////////////////////////////////////////////////////
    mChannelID get_newChannelID();

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Set filters on the logger.
    ///
    /// \param a_filter The channels that will be allowed
    ///////////////////////////////////////////////////////////////////////////
    void set_filter(mChannelFilter a_filter);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Disable channels of the logger.
    ///
    /// \param a_filter The channels to disable if enabled
    ///////////////////////////////////////////////////////////////////////////
    void disable_channels(mChannelFilter a_filter);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Enable channels of the logger.
    ///
    /// \param a_filter The channels to Enable if disabled
    ///////////////////////////////////////////////////////////////////////////
    void enable_channels(mChannelFilter a_filter);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Print things in the logs using a specified channel
    ///
    /// \tparam t_Severity The severity of the things to log
    /// \tparam t_Args The types of the things to log
    /// \param a_args The things to log
    /// \param a_channel The channel ID to print to.
    ///////////////////////////////////////////////////////////////////////////
    template <mSeverityType t_Severity, typename... t_Args>
    void print_toChannel(uint64_t a_channel, t_Args... a_args);

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Print things in the logs
    ///
    /// \tparam t_Severity The severity of the things to log
    /// \tparam t_Args The types of the things to log
    /// \param a_args The things to log
    ///////////////////////////////////////////////////////////////////////////
    template <mSeverityType t_Severity, typename... t_Args>
    void print(t_Args... a_args);

   private:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Get the time of the day into a string format
    ///////////////////////////////////////////////////////////////////////////
    std::string get_time();

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Construct the header of the log line
    ///////////////////////////////////////////////////////////////////////////
    std::string get_loglineHeader();

    ///////////////////////////////////////////////////////////////////////////
    /// \brief End of the variadic function.
    ///////////////////////////////////////////////////////////////////////////
    void print_impl();

    ///////////////////////////////////////////////////////////////////////////
    /// \brief Variadic function implementing the writing of the output.
    ///
    /// \tparam t_First Type of the first thing to print.
    /// \tparam t_Rest The rest of the types of things to print.
    /// \param a_parm1 The first thing to print.
    /// \param a_param The rest of things to print.
    ///////////////////////////////////////////////////////////////////////////
    template <typename t_First, typename... t_Rest>
    void print_impl(t_First a_parm1, t_Rest... a_parm);

   private:
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Mutex preventing multiple writes to the output file
    ///////////////////////////////////////////////////////////////////////////
    std::mutex        m_mutexWrite;
    std::stringstream m_logStream;         //!< The current line to write
    t_LogPolicy*      m_policy = nullptr;  //!< Pointer to the logpolicy to use
    unsigned          m_lineNumber = 0;    //!< Number of the current log line

    ///////////////////////////////////////////////////////////////////////////
    /// \brief The current filter used to filter out specific channels
    ///////////////////////////////////////////////////////////////////////////
    mChannelFilter m_filter        = g_allChannelsFilter;
    mChannelID     m_nextChannelID;  //!< The next channel id to attribute
};

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
template <typename t_LogPolicy>
mLogger<t_LogPolicy>::mLogger(const std::string& a_name)
{
    m_filter        = -1;
    m_lineNumber    = 0;
    m_policy        = new t_LogPolicy;
    if (!m_policy)
    {
        throw std::runtime_error(
            "LOGGER: Unable to create the logger instance");
    }
    m_policy->open_ostream(a_name);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
template <typename t_LogPolicy>
mLogger<t_LogPolicy>::~mLogger()
{
    if (m_policy)
    {
        m_policy->close_ostream();
        delete m_policy;
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
template <typename t_LogPolicy>
mChannelID mLogger<t_LogPolicy>::get_newChannelID()
{
    // TODO : Fix this ID generation problem
    if(m_nextChannelID == 0)
    {
        m_nextChannelID = 1;
    }
    return m_nextChannelID <<= 1;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
template <typename t_LogPolicy>
void mLogger<t_LogPolicy>::set_filter(const mChannelFilter a_filter)
{
    m_filter = a_filter;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
template <typename t_LogPolicy>
void mLogger<t_LogPolicy>::disable_channels(const mChannelFilter a_filter)
{
    m_filter = m_filter & ~a_filter;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
template <typename t_LogPolicy>
void mLogger<t_LogPolicy>::enable_channels(const mChannelFilter a_filter)
{
    m_filter = m_filter | a_filter;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
template <typename t_LogPolicy>
template <mSeverityType t_Severity, typename... t_Args>
void mLogger<t_LogPolicy>::print_toChannel(uint64_t a_channel, t_Args... a_args)
{
    if (a_channel & m_filter)
    {
        print<t_Severity>(a_args...);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
template <typename t_LogPolicy>
template <mSeverityType t_Severity, typename... t_Args>
void mLogger<t_LogPolicy>::print(t_Args... a_args)
{
    m_mutexWrite.lock();
    switch (t_Severity)
    {
        case mSeverityType::debug: m_logStream << "<DEBUG> :"; break;
        case mSeverityType::warning: m_logStream << "<WARNING> :"; break;
        case mSeverityType::error: m_logStream << "<ERROR> :"; break;
    };
    print_impl(a_args...);
    m_mutexWrite.unlock();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename t_LogPolicy>
std::string mLogger<t_LogPolicy>::get_time()
{
    time_t raw_time;
    time(&raw_time);

    struct tm* timeInfo;
#ifdef M_WIN32
    timeInfo = localtime(&raw_time);
#else
    timeInfo = localtime(&raw_time);
#endif
    mChar buffer[80];
    strftime(buffer, 80, "%d/%m/%y - %H/%M/%S", timeInfo);

    std::string time_str(buffer);
    return time_str.substr(0, time_str.size() - 1);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename t_LogPolicy>
std::string mLogger<t_LogPolicy>::get_loglineHeader()
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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename t_LogPolicy>
void mLogger<t_LogPolicy>::print_impl()
{
    m_policy->write(get_loglineHeader() + m_logStream.str());
    m_logStream.str("");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
template <typename t_LogPolicy>
template <typename t_First, typename... t_Rest>
void mLogger<t_LogPolicy>::print_impl(t_First a_parm1, t_Rest... a_parm)
{
    m_logStream << a_parm1;
    print_impl(a_parm...);
}

}  // namespace m::logging
///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////