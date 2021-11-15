#pragma once

#include <DirectXMath.h>

#include "Kernel/MathTypes.hpp"
#include "Kernel/Types.hpp"

namespace m::render
{
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
class mCamera
{
   public:
    mFloat get_vFoV() const { return m_vFoV; }
    void   set_vFoV(mFloat a_vFoV);

    void                     set_fromLookAt(math::mVec3 const& a_position,
                                            math::mVec3 const& a_lookAt, math::mVec3 const& a_up);
    DirectX::XMMATRIX const& get_viewMatrix();
    DirectX::XMMATRIX        get_projectionMatrix(mFloat a_aspectRatio);

   private:
    DirectX::XMMATRIX m_viewMatrix;
    DirectX::XMMATRIX m_matrix;
    mFloat            m_vFoV         = 45.0f;   // Vertical field of view
    mFloat            m_aspectRatio  = 0.0f;    // Aspect ratio
    mFloat            m_near         = 0.1f;    // Near clip distance
    mFloat            m_far          = 100.0f;  // Far clip distance
    mBool             m_orthographic = false;
};
}  // namespace m::render