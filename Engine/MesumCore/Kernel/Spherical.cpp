#include "Spherical.hpp"

#include <numbers>

namespace m::math
{
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mSpherical::mSpherical() {}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mSpherical::mSpherical(mFloat a_latitude, mFloat a_longitude, mFloat a_radius)
{
    set_latitude(a_latitude);
    set_longitude(a_longitude);
    set_radius(a_radius);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSpherical::set_latitude(mFloat a_latitude)
{
    m_latitude = std::clamp(a_latitude, std::numbers::pi_v<mFloat> * -0.5f,
                            std::numbers::pi_v<mFloat> *
                                0.5f);  // TODO faire une constante quand meme
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSpherical::add_latitude(mFloat a_deltaLatitude)
{
    set_latitude(m_latitude + a_deltaLatitude);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSpherical::set_longitude(mFloat a_longitude)
{
    m_longitude = a_longitude;

    // TODO faire un angleClamp
    while (m_longitude < -std::numbers::pi_v<mFloat>)
    {
        m_longitude += 2.0f * std::numbers::pi_v<mFloat>;
    }
    while (m_longitude >= std::numbers::pi_v<mFloat>)  // TODO >= ?
    {
        m_longitude -= 2.0f * std::numbers::pi_v<mFloat>;
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSpherical::add_longitude(mFloat a_deltaLongitude)
{
    set_longitude(m_longitude + a_deltaLongitude);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSpherical::set_radius(mFloat a_radius)
{
    if (a_radius > 0.0f)  // TODO epsilon
    {
        m_radius = a_radius;
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mSpherical::compute_cartesianCoordinates(
    mVec3& a_outCartesianCoordinates) const
{
    mFloat cosLatitude  = cos(m_latitude);
    mFloat sinLatitude  = sin(m_latitude);
    mFloat cosLongitude = cos(m_longitude);
    mFloat sinLongitude = sin(m_longitude);

    a_outCartesianCoordinates.x = sinLongitude * cosLatitude * m_radius;
    a_outCartesianCoordinates.y = sinLatitude * m_radius;
    a_outCartesianCoordinates.z = cosLongitude * cosLatitude * m_radius;
}
}  // namespace m::math