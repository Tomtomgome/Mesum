#ifndef M_LOG_POLICIES
#define M_LOG_POLICIES
#pragma once
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>

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

		//! Interface defining a logging policy .
		/*!
			The policy difines where the logs are outputed (file, stream, etc...).
		*/
		class ILogPolicy
		{
		public:
			virtual ~ILogPolicy() {};

			//! Open the output stream.
			/*!
				\param	a_name	the name of the stream.
			*/
			virtual void	open_ostream(const std::string& a_name) = 0;
			//! close the output stream
			virtual void	close_ostream() = 0;
			//! write into the output steam
			/*!
				\param	a_msg		the message to output.
			*/
			virtual void	write(const std::wstring& a_msg) = 0;
		};

		//! Log policy outputing into a file
		class FileLogPolicy : public ILogPolicy
		{
		public:
			//! Construct a file logging policy.
			/*!
				This creates a new ofstream object.
			*/
			FileLogPolicy() : m_outStream(new std::wofstream) {}
			//! Destroy a file logging policy.
			/*!
				This destroy the ofstream object.
			*/
			virtual ~FileLogPolicy();
			//! Open the output file.
			/*!
				\param	a_name	the name of the file.
			*/
			void	open_ostream(const std::string& a_name)	override;
			//! close the output file
			void	close_ostream()							override;
			//! write into the output file
			/*!
				\param	a_msg		the message to output.
			*/
			void	write(const std::wstring& a_msg)			override;

		private:
			//! The output file.
			std::unique_ptr<std::wofstream> m_outStream;
		};

		//! Log policy outputing into the standard output
		class StdcoutLogPolicy : public ILogPolicy
		{
		public:
			//! Construct the cout logging policy.
			StdcoutLogPolicy();
			//! Destroy a file logging policy.
			/*!
				This destroy the ofstream object.
			*/
			virtual ~StdcoutLogPolicy();
			//! Open the output stream.
			/*!
				\param	a_name	the name of the file.
			*/
			void	open_ostream(const std::string& a_name)	override;
			//! close the output stream
			void	close_ostream()							override;
			//! write into the output stream
			/*!
				\param	a_msg		the message to output.
			*/
			void	write(const std::wstring& a_msg)			override;

		};
	}
}
#endif //M_LOG_POLICIES
/** @}*/