#pragma once

#include "Kernel/Types.hpp"
#include "Kernel/MathTypes.hpp"

#include "HgtPosition.hpp"
#include "Mesh.hpp"

#include <vector>
#include <string>
#include <map>

using namespace m;

class mSrtmHm
{
   public:
    void ReadAllHgtFiles();

    static void         ReadHgtFile(std::string        a_rsFilePath,
                                    std::vector<mU16>& a_rv2HgtData);
    static math::mIVec2 GetFileCoordinatesFromFileName(
        std::string const& a_fileName);
    mI16 GetAltitudeFromHgtPosition(mHgtPosition const& a_rHgtPosition) const;
    math::mVec3 AngleToPosition(math::mVec2 a_rv2Angle, math::mVec2 a_rv2Center,
                                mI16 a_iAltitude) const;
    m::render::mMesh _UpdateMesh();
    void             ComputeZoneAngle(math::mVec2& a_rv2ZoneAngleBottomLeft,
                                      math::mVec2& a_rv2ZoneAngleSize,
                                      math::mVec2& a_rv2ZoneAngleCenter) const;

    void SelfCheck();

    //----------------------------------------------
    // Data
    //----------------------------------------------
    math::mVec2 m_v2ZoneTopLeft =
        math::mVec2({45.91217545981639, 6.160962321129078});
    math::mVec2 m_v2ZoneBottomRight =
        math::mVec2({45.87112092462624, 6.210503661706545});

    mFloat m_fEarthRadius = 6371000.0f;  // In meters
    mFloat m_fWorldScale  = 0.001f;

    mFloat m_fCurveMultiplier = 1.0f;

    //----------------------------------------------
    // Runtime
    //----------------------------------------------
    std::map<math::mIVec2, std::vector<mU16>> m_mHgtData;
};