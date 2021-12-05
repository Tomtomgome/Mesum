#include "Profile.hpp"

namespace m::profile
{
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mRAIITiming::mRAIITiming(mIProfiler& a_parent) : m_pParent(&a_parent)
{
    m_start = std::chrono::high_resolution_clock::now();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mRAIITiming::~mRAIITiming()
{
    mExpect(m_pParent != nullptr);
    auto end = std::chrono::high_resolution_clock::now();
    m_pParent->add_timing(end - m_start);
}
}  // namespace m::profile