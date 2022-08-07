#include "Camera.hpp"

#include "Kernel/MatHelpers.hpp"

namespace m::render
{
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCamera::set_vFov(mFloat a_vFov)
{
    if (m_vFov != a_vFov)
    {
        m_vFov = a_vFov;
        // recompute
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCamera::set_fromLookAt(math::mVec3 const& a_position,
                             math::mVec3 const& a_lookAt,
                             math::mVec3 const& a_up)
{
    // Update the view matrix.
    m_viewMatrix = generate_lookAtLH(a_position, a_lookAt, a_up);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
math::mMat4x4 const& mCamera::get_viewMatrix()
{
    return m_viewMatrix;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
math::mMat4x4 mCamera::get_projectionMatrix(mFloat a_aspectRatio)
{
    // Update the projection matrix.
    return m::math::generate_projectionPerspectiveLH(m_vFov, a_aspectRatio, m_near, m_far);
}
}  // namespace m::render