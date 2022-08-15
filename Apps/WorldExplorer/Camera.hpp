#pragma once

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
    mFloat get_vFov() const { return m_vFov; }
    void   set_vFov(mFloat a_vFov);

    void                 set_fromLookAt(math::mVec3 const& a_position,
                                        math::mVec3 const& a_lookAt, math::mVec3 const& a_up);
    math::mMat4x4 const& get_viewMatrix();
    math::mMat4x4        get_projectionMatrix(mFloat a_aspectRatio);

   private:
    math::mMat4x4 m_viewMatrix;
    math::mMat4x4 m_matrix;
    mFloat        m_vFov         = 45.0f;   // Vertical field of view
    mFloat        m_aspectRatio  = 0.0f;    // Aspect ratio
    mFloat        m_near         = 0.1f;    // Near clip distance
    mFloat        m_far          = 100.0f;  // Far clip distance
    mBool         m_orthographic = false;
};
}  // namespace m::render