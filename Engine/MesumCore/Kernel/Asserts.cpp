#include <Asserts.hpp>

namespace m
{
    extern MesumCoreApi const logging::ChannelID ASSERT_ID = mLOG_GET_ID();

	void manage_simple_assert(Bool a_condition,
		const Int a_lineNumber, const Char* a_file)
	{
		if (!a_condition) {
			mLOG_WARN_TO(ASSERT_ID,
				"Triggered assertion from file ", a_file,
				" at Line ", a_lineNumber);
		}
	}

	void manage_blocking_assert(Bool a_condition,
		const Int a_lineNumber, const Char* a_file)
	{
		if (!a_condition) {
			mLOG_ERR_TO(ASSERT_ID,
				"Triggered herd assertion from file ", a_file,
				" at Line ", a_lineNumber);
			mInterrupt;
		}
	}
}