#pragma once
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>

/** \addtogroup Log
*	@{
*/
//! Namespace grouping all logging tools.
namespace logging {

	//! Interface defining a logging policy .
	/*!
		The policy difines where the logs are outputed (file, stream, etc...).
	*/
	class log_policy_interface
	{
	public:
		//! Open the output stream.
		/*!
			\param	name	the name of the stream.
		*/
		virtual void	open_ostream(const std::string& name) = 0;
		//! close the output stream
		virtual void	close_ostream() = 0;
		//! write into the output steam
		/*!
			\param	msg		the message to output.
		*/
		virtual void	write(const std::string& msg) = 0;
	};

	//! Log policy outputing into a file
	class file_log_policy : public log_policy_interface
	{
	public:
		//! Construct a file logging policy.
		/*!
			This creates a new ofstream object.
		*/
		file_log_policy() : out_stream(new std::ofstream) {}
		//! Destroy a file logging policy.
		/*!
			This destroy the ofstream object.
		*/
		~file_log_policy();
		//! Open the output file.
		/*!
			\param	name	the name of the file.
		*/
		void	open_ostream(const std::string& name)	override;
		//! close the output file
		void	close_ostream()							override;
		//! write into the output file
		/*!
			\param	msg		the message to output.
		*/
		void	write(const std::string& msg)			override;

	private:
		//! The output file.
		std::unique_ptr<std::ofstream> out_stream;
	};

	//! Log policy outputing into the standard output
	class stdcout_log_policy : public log_policy_interface
	{
	public:
		//! Construct the cout logging policy.
		stdcout_log_policy();
		//! Destroy a file logging policy.
		/*!
			This destroy the ofstream object.
		*/
		~stdcout_log_policy();
		//! Open the output stream.
		/*!
			\param	name	the name of the file.
		*/
		void	open_ostream(const std::string& name)	override;
		//! close the output stream
		void	close_ostream()							override;
		//! write into the output stream
		/*!
			\param	msg		the message to output.
		*/
		void	write(const std::string& msg)			override;

	};
}

/** @}*/