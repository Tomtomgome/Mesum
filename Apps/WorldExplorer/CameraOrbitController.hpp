#pragma once

#include "Camera.hpp"
#include "Kernel/MathTypes.hpp"
#include "Kernel/Spherical.hpp"
#include "Kernel/StateInputManager.hpp"
#include "Kernel/Types.hpp"
#include "Kernel/Quaternion.hpp"

using namespace m::math;

namespace m::game
{
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
class mCameraOrbitController
{
   public:
    void update(std::chrono::steady_clock::duration const& a_deltaTime,
                input::mStateInputManager const&           a_inputManager);
    void update_camera(render::mCamera& a_camera) const;

   private:
    mVec3      m_pivot = mVec3({0.0f, 0.0f, -10.0f});
    mSpherical m_orbit;
    mFloat     m_halfViewHeight = 5.0f;
    mFloat     m_vFoV           = 70.0f;

    mFloat m_moveSpeed   = 5.0f;
    mFloat m_rotateSpeed = 1.0f;
};
}  // namespace m::game