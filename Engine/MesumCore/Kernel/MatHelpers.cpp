#include "MatHelpers.hpp"

namespace m::math
{
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mMat4x4 generate_translation(mFloat a_x, mFloat a_y, mFloat a_z)
{
    return {1.0f, 0.0f, 0.0f, a_x,
            0.0f, 1.0f, 0.0f, a_y,
            0.0f, 0.0f, 1.0f, a_z,
            0.0f, 0.0f, 0.0f, 1.0f};
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mMat4x4 generate_projectionOrthoLH(mFloat a_width, mFloat a_height,
                                   mFloat a_near, mFloat a_far)
{
    mFloat range = 1.0f / (a_far - a_near);

    return {2.0f / a_width, 0.0f, 0.0f, 0.0f,
            0.0f,  2.0f / a_height, 0.0f, 0.0f,
            0.0f, 0.0f, range, -range * a_near,
            0.0f, 0.0f, 0.0f, 1.0f};
}
};  // namespace m::math