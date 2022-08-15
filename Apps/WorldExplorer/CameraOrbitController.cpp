#include "CameraOrbitController.hpp"

const m::logging::mChannelID m_CAM_LOG_ID = mLog_getId();

namespace m::game
{
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCameraOrbitController::update(
    std::chrono::steady_clock::duration const& a_deltaTime,
    input::mStateInputManager const&           a_inputManager)
{
    mDouble deltaTime = std::chrono::duration<mDouble>(a_deltaTime).count();
    if (deltaTime > 0.1)
    {
        deltaTime = 0.1;  // clamp delta time! TODO
    }

    // rotate

    mVec2 deltaRotate;

    if (a_inputManager.get_keyState(input::mKey::keyLeft) ==
        input::mInputType::pressed)
    {
        deltaRotate.x = -1.0f;
    }
    else if (a_inputManager.get_keyState(input::mKey::keyRight) ==
             input::mInputType::pressed)
    {
        deltaRotate.x = 1.0f;
    }

    if (a_inputManager.get_keyState(input::mKey::keyDown) ==
        input::mInputType::pressed)
    {
        deltaRotate.y = -1.0f;
    }
    else if (a_inputManager.get_keyState(input::mKey::keyUp) ==
             input::mInputType::pressed)
    {
        deltaRotate.y = 1.0f;
    }

    m_orbit.add_longitude(deltaRotate.x * m_rotateSpeed * deltaTime);
    m_orbit.add_latitude(deltaRotate.y * m_rotateSpeed * deltaTime);

    // move

    // TODO crash si j'appuie sur mute
    mVec3 deltaMove;

    if (a_inputManager.get_keyState(input::mKey::keyQ) ==
        input::mInputType::pressed)
    {
        deltaMove.x = -1.0f;
    }
    else if (a_inputManager.get_keyState(input::mKey::keyD) ==
             input::mInputType::pressed)
    {
        deltaMove.x = 1.0f;
    }

    if (a_inputManager.get_keyState(input::mKey::keyS) ==
        input::mInputType::pressed)
    {
        deltaMove.z = -1.0f;
    }
    else if (a_inputManager.get_keyState(input::mKey::keyZ) ==
             input::mInputType::pressed)
    {
        deltaMove.z = 1.0f;
    }

    if (length(deltaMove) > 1.0f)  // TODO self normalize
    {
        deltaMove = normalized(deltaMove);  // TODO le retour par copie bon ?!
    }

    mVec3 coordinates;
    m_orbit.compute_cartesianCoordinates(coordinates);
    coordinates.y = 0.0f; // TODO kk faudrait plutôt get uniquement le vecteur de longitude comme vecteur de rotation
    coordinates = normalized(coordinates);

    mVec3 forward{0.0f, 0.0f, 1.0f};
    deltaMove = mQuaternion(forward, coordinates) * deltaMove;

    // up / down

    if (a_inputManager.get_keyState(input::mKey::keyA) ==
        input::mInputType::pressed)
    {
        deltaMove.y = -1.0f;
    }
    else if (a_inputManager.get_keyState(input::mKey::keyE) ==
             input::mInputType::pressed)
    {
        deltaMove.y = 1.0f;
    }

    deltaMove *= m_moveSpeed * deltaTime;

    m_pivot += deltaMove;


    /* 16 left shift          16 right shift        173 mute TODO crash */
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mCameraOrbitController::update_camera(render::mCamera& a_camera) const
{
    mVec3 lookAt;
    m_orbit.compute_cartesianCoordinates(lookAt);

    /* mLog_infoTo(m_CAM_LOG_ID, " m_orbit latitude: ", m_orbit.get_latitude(), "
     * m_orbit longitude: ", m_orbit.get_longitude()); */

    a_camera.set_fromLookAt(m_pivot, (m_pivot + lookAt),
                            mVec3({0.0f, 1.0f, 0.0f}));
}
}  // namespace m::game