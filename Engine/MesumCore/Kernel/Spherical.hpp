#pragma once

#include "MathTypes.hpp"
#include "Types.hpp"

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////
namespace m::math
{
class mSpherical
{
   public:
    mSpherical();
    mSpherical(mFloat a_latitude, mFloat a_longitude, mFloat a_radius);

    [[nodiscard]] mFloat get_latitude() const { return m_latitude; }
    void                 set_latitude(mFloat a_latitude);
    void                 add_latitude(mFloat a_deltaLatitude);

    [[nodiscard]] mFloat get_longitude() const { return m_longitude; }
    void                 set_longitude(mFloat a_longitude);
    void                 add_longitude(mFloat a_deltaLongitude);

    [[nodiscard]] mFloat get_radius() const { return m_radius; }
    void                 set_radius(mFloat a_radius);

    void compute_cartesianCoordinates(mVec3& a_outCartesianCoordinates) const;

   private:
    mFloat m_latitude  = 0.0f;
    mFloat m_longitude = 0.0f;
    mFloat m_radius    = 1.0f;
};
}  // namespace m::math

///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////
