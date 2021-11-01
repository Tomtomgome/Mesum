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
void manage_assert(mBool a_condition, const mInt a_lineNumber, const mChar* a_file,
                   const mChar* a_message,
                   mBool a_interrupt)
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