#include <Asserts.hpp>

namespace m
{
MesumCoreApi const logging::mChannelID ASSERT_ID = mLog_getId();

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void manage_assert(Bool a_condition, const Int a_lineNumber, const Char* a_file,
                   const Char* a_message, Bool a_interrupt)
{
    if (!a_condition)
    {
        mLog_warningTo(ASSERT_ID, "Triggered assertion from file ", a_file,
                     " at Line ", a_lineNumber);
        if (a_interrupt)
        {
            mInterrupt;
        }
    }
}
}  // namespace m