#pragma once

#include "Vec.hpp"
#include "Mat.hpp"

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////
namespace m::math
{
using mVec2 = mVec<mFloat, 2>;
using mVec3 = mVec<mFloat, 3>;
using mVec4 = mVec<mFloat, 4>;

using mDVec2 = mVec<mDouble, 2>;
using mDVec3 = mVec<mDouble, 3>;
using mDVec4 = mVec<mDouble, 4>;

using mIVec2 = mVec<mInt, 2>;
using mIVec3 = mVec<mInt, 3>;
using mIVec4 = mVec<mInt, 4>;

using mUIVec2 = mVec<mUInt, 2>;
using mUIVec3 = mVec<mUInt, 3>;
using mUIVec4 = mVec<mUInt, 4>;

using mMat4x4 = mMat<mFloat, 4, 4>;
};  // namespace m::math
///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////