#include <Log.hpp>

namespace logging {
	//------------------------------------------------------------
	//------------------------------------------------------------
	//------------------------------------------------------------
	void file_log_policy::open_ostream(const std::wstring& name) {
		out_stream->open(name.c_str(), std::ios_base::binary | std::ios_base::out);
		if (!out_stream->is_open())
		{
			throw std::runtime_error("LOGGER: unable to open an output stream");
		}
	}

	void file_log_policy::close_ostream() {
		if (out_stream)
		{
			out_stream->close();
		}
	}

	void file_log_policy::write(const std::wstring& msg) {
		(*out_stream) << msg << std::endl;
	}

	file_log_policy::~file_log_policy()
	{
		if (out_stream)
		{
			close_ostream();
		}
	}

	//------------------------------------------------------------
	//------------------------------------------------------------
	//------------------------------------------------------------
	stdcout_log_policy::stdcout_log_policy(){
	}

	void stdcout_log_policy::open_ostream(const std::wstring& name) {
	}

	void stdcout_log_policy::close_ostream() {
	}

	void stdcout_log_policy::write(const std::wstring& msg) {
		std::wcout << msg << std::endl;
	}

	stdcout_log_policy::~stdcout_log_policy(){
	}
}