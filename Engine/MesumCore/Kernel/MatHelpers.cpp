#include "MatHelpers.hpp"

#include "MatGlm.hpp"

namespace m::math
{
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mMat4x4 generate_translationMatrix(mFloat a_x, mFloat a_y, mFloat a_z)
{
    return {1.0f, 0.0f, 0.0f, a_x, 0.0f, 1.0f, 0.0f, a_y,
            0.0f, 0.0f, 1.0f, a_z, 0.0f, 0.0f, 0.0f, 1.0f};
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mMat4x4 generate_projectionOrthoLH(mFloat a_width, mFloat a_height,
                                   mFloat a_near, mFloat a_far)
{
    mFloat range = 1.0f / (a_far - a_near);

    return {2.0f / a_width, 0.0f,
            0.0f,           2.0f / a_width,
            0.0f,           2.0f / a_height,
            0.0f,           2.0f / a_height,
            0.0f,           0.0f,
            range,          -range * a_near,
            0.0f,           0.0f,
            0.0f,           1.0f};
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mMat4x4 generate_projectionPerspectiveLH(mFloat a_vFov, mFloat a_aspectRatio,
                                         mFloat a_near, mFloat a_far)
{
    glm::mat4 result = glm::perspectiveLH(glm::radians(a_vFov),
                                             a_aspectRatio, a_near, a_far);
    return M_MAT4_GLM_TO_MESUM(result);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mMat4x4 generate_scaleMatrix(mFloat a_x, mFloat a_y, mFloat a_z)
{
    return {a_x,  0.0f, 0.0f, 0.0f, 0.0f, a_y,  0.0f, 0.0f,
            0.0f, 0.0f, a_z,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mMat4x4 generate_lookAtLH(mVec3 const& a_position, mVec3 const& a_lookAt,
                          mVec3 const& a_up)
{
    glm::mat4 result =
        glm::lookAtLH(M_VEC3_MESUM_TO_GLM(a_position),
                      M_VEC3_MESUM_TO_GLM(a_lookAt), M_VEC3_MESUM_TO_GLM(a_up));

    return M_MAT4_GLM_TO_MESUM(result);
}
};  // namespace m::math