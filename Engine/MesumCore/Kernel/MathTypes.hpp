#pragma once

#include <Vec.hpp>
#include <cstring>

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////
namespace m::math
{
using mVec2 = Vec<mFloat, 2>;
using mVec3 = Vec<mFloat, 3>;
using mVec4 = Vec<mFloat, 4>;

using mDVec2 = Vec<mDouble, 2>;
using mDVec3 = Vec<mDouble, 3>;
using mDVec4 = Vec<mDouble, 4>;

using mIVec2 = Vec<mInt, 2>;
using mIVec3 = Vec<mInt, 3>;
using mIVec4 = Vec<mInt, 4>;

using mUIVec2 = Vec<mUInt, 2>;
using mUIVec3 = Vec<mUInt, 3>;
using mUIVec4 = Vec<mUInt, 4>;
};  // namespace m::math
///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////