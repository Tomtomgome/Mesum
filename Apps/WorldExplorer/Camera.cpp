#include "Camera.hpp"

using namespace DirectX;

namespace m::render
{
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCamera::set_vFoV(mFloat a_vFoV)
{
    if (m_vFoV != a_vFoV)
    {
        m_vFoV = a_vFoV;
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
    const XMVECTOR dxPosition =
        XMVectorSet(a_position.x, a_position.y, a_position.z, 1);
    const XMVECTOR dxLookAt =
        XMVectorSet(a_lookAt.x, a_lookAt.y, a_lookAt.z, 1);
    const XMVECTOR dxUp = XMVectorSet(a_up.x, a_up.y, a_up.z, 0);

    // Update the view matrix.
    m_viewMatrix = XMMatrixLookAtLH(dxPosition, dxLookAt, dxUp);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
DirectX::XMMATRIX const& mCamera::get_viewMatrix()
{
    return m_viewMatrix;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
DirectX::XMMATRIX mCamera::get_projectionMatrix(mFloat a_aspectRatio)
{
    // Update the projection matrix.
    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(m_vFoV), a_aspectRatio, m_near, m_far);
    return projectionMatrix;
}
}  // namespace m::render