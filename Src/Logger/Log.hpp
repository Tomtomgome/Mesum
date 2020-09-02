#pragma once
#include <string>
#include <memory>
#include <fstream>
#include <mutex>
#include <sstream>

#include <LogPolicies.hpp>

/** \addtogroup Log
*	@{
*/
//! Namespace grouping all logging tools.
namespace logging {
	//! Defining what a ChannelID is.
	using ChannelID = uint64_t;
	//! Defining what a ChannelFilter is.
	using ChannelFilter = uint64_t;

	//! Enumeration of the logginh severity levels.
	enum severity_type
	{
		debug = 1,	/*!< sets a debug severity level.*/
		error,		/*!< sets a error severity level.*/
		warning		/*!< sets a warning severity level.*/
	};

	//! Thread safe logger class used to write logs.
	/*!
		\tparam		log_policy	the logging policy used by the logger.
	*/
	template< typename log_policy >
	class logger
	{
	public:
		//! Construct a logger.
		/*!
			\param	name	the output stream name.
		*/
		logger(const std::string& name);

		//! Get a new channel ID
		ChannelID getNewChannelID();

		//! Set filters on the logger.
		/*!
			\param	filters		The channels that will be allowed.
		*/
		void setFilter(const ChannelFilter filters);

		//! Disable channels of the logger.
		/*!
			\param	filters		The channels to disable if enabled.
		*/
		void disableChannels(const ChannelFilter filters);

		//! Enable channels of the logger.
		/*!
			\param	filters		The channels to enable if disabled.
		*/
		void enableChannels(const ChannelFilter filters);

		//! Print things in the logs using a specified channel
		/*!
			\tparam	severity	the severity of the things to log.
			\tparam	Args		the list of things to log.
			\param	args		the things to log.
			\param	channel		the channel number to print to.
		*/
		template< severity_type severity, typename...Rest >
		void printToChannel(uint64_t channel, Rest...args);
		//! Print things in the logs
		/*!
			\tparam	severity	the severity of the things to log.
			\tparam	Args		the list of things to log.
			\param	args		the things to log.
		*/
		template< severity_type severity, typename...Args >
		void print(Args...args);

		//! Destroy a logger.
		~logger();
	private:
		//!	Get the time of the day into a string format.
		std::string get_time();
		//!	Construct the header of the log line.
		std::string get_logline_header();
		
		//!	The current line to write.
		std::stringstream log_stream;
		//!	Pointer to the logpolicy to use.
		log_policy* policy;
		//! Mutex preventing multiple writes to the output file.
		std::mutex write_mutex;

		//! End of the variadic function.
		void print_impl();

		//! Variadic function implementing the writing of the output.
		/*!
			\tparam	First	type of the first thing to print.
			\tparam Rest	the rest of the types of things to print.
			\param	First	the first thing to print.
			\param	param	the rest of things to print.
		*/
		template<typename First, typename...Rest>
		void print_impl(First parm1, Rest...parm);

	private:
		//! Number of the current log line.
		unsigned log_line_number;
		//! The filter used to filter out specific channels
		ChannelFilter filter = -1;
		//! The next channel id to attribute
		ChannelID nextChannelID = 1;

	};

	template< typename log_policy >
	template< severity_type severity, typename...Rest >
	void logger< log_policy >::printToChannel(uint64_t channel, Rest...args) {
		if (channel & filter) {
			print< severity >(args...);
		}
	}

	template< typename log_policy >
	template< severity_type severity, typename...Args >
	void logger< log_policy >::print(Args...args)
	{
		write_mutex.lock();
		switch (severity)
		{
		case severity_type::debug:
			log_stream << "<DEBUG> :";
			break;
		case severity_type::warning:
			log_stream << "<WARNING> :";
			break;
		case severity_type::error:
			log_stream << "<ERROR> :";
			break;
		};
		print_impl(args...);
		write_mutex.unlock();
	}

	template< typename log_policy >
	void logger< log_policy >::print_impl()
	{
		policy->write(get_logline_header() + log_stream.str());
		log_stream.str("");
	}

	template< typename log_policy >
	template<typename First, typename...Rest >
	void logger< log_policy >::print_impl(First parm1, Rest...parm)
	{
		log_stream << parm1;
		print_impl(parm...);
	}

	template< typename log_policy >
	std::string logger< log_policy >::get_time()
	{
		std::string time_str;
		time_t raw_time;
		time(&raw_time);
		time_str = ctime(&raw_time);
		//without the newline character
		return time_str.substr(0, time_str.size() - 1);
	}

	template< typename log_policy >
	std::string logger< log_policy >::get_logline_header()
	{
		std::stringstream header;
		header.str("");
		header.fill('0');
		header.width(7);
		header << log_line_number++ << " < " << get_time() << " - ";
		header.fill('0');
		header.width(7);
		header << clock() << " > ~ ";
		return header.str();
	}

	template< typename log_policy >
	logger< log_policy >::logger(const std::string& name)
	{
		log_line_number = 0;
		policy = new log_policy;
		if (!policy)
		{
			throw std::runtime_error("LOGGER: Unable to create the logger instance");
		}
		policy->open_ostream(name);
	}

	template< typename log_policy >
	logger< log_policy >::~logger()
	{
		if (policy)
		{
			policy->close_ostream();
			delete policy;
		}
	}

	template<typename log_policy>
	ChannelID logger< log_policy >::getNewChannelID() {
		return nextChannelID <<= 1;
	}

	template<typename log_policy>
	void logger< log_policy >::setFilter(const ChannelFilter filters) {
		filter = filters;
	}

	template<typename log_policy>
	void logger< log_policy >::disableChannels(const ChannelFilter filters) {
		filter = filter & ~filters;
	}

	template<typename log_policy>
	void logger< log_policy >::enableChannels(const ChannelFilter filters) {
		filter = filter | filters;
	}

}

/** @}*/