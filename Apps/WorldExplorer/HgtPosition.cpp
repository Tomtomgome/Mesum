#include "HgtPosition.hpp"

mInt const g_uSrtmPoint   = 3601;  // 5
mInt const g_uSrtmSegment = 3600;  // 4

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mFloat oFloor(mFloat a_fVal)
{
    return floorf(a_fVal);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mInt oFloorToInt(mFloat a_fVal)
{
    return mInt(floorf(a_fVal));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mUInt oFloorToUInt(mFloat a_fVal)
{
    return mUInt(floorf(a_fVal));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mFloat oFractional(mFloat a_fVal)
{
    mFloat fInteger;
    return modff(a_fVal, &fInteger);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mFloat oFractionalPositive(mFloat a_fVal)
{
    if (a_fVal >= 0.0f)
    {
        return oFractional(a_fVal);
    }
    else
    {
        return a_fVal - oFloor(a_fVal);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mHgtPosition::mHgtPosition(math::mIVec2  a_ri2FileCoordinates,
                           math::mUIVec2 a_ru2Tile)
    : m_i2FileCoordinates(a_ri2FileCoordinates),
      m_u2TileIndex(a_ru2Tile)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mBool mHgtPosition::operator==(mHgtPosition const& a_rVal) const
{
    return (m_i2FileCoordinates == a_rVal.m_i2FileCoordinates) &&
           (m_u2TileIndex == a_rVal.m_u2TileIndex);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mHgtPosition::SetFromAngle(math::mVec2 a_rv2Angle)
{
    m_i2FileCoordinates =
        math::mIVec2({oFloorToInt(a_rv2Angle.x), oFloorToInt(a_rv2Angle.y)});

    m_u2TileIndex = math::mUIVec2(
        {oFloorToUInt(oFractionalPositive(a_rv2Angle.x) * g_uSrtmPoint),
         oFloorToUInt(oFractionalPositive(a_rv2Angle.y) * g_uSrtmPoint)});

    mAssert((m_u2TileIndex.x) < g_uSrtmPoint &&
            (m_u2TileIndex.y) < g_uSrtmPoint);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mHgtPosition::Move(mInt a_uMoveX, mInt a_uMoveY)
{
    m_i2FileCoordinates +=
        math::mIVec2({a_uMoveX / g_uSrtmSegment, a_uMoveY / g_uSrtmSegment});

    m_u2TileIndex +=
        math::mUIVec2({static_cast<mUInt>(a_uMoveX % g_uSrtmSegment),
                       static_cast<mUInt>(a_uMoveY % g_uSrtmSegment)});

    while (m_u2TileIndex.x >= g_uSrtmSegment)
    {
        m_u2TileIndex.x -= g_uSrtmSegment;
        m_i2FileCoordinates.x++;
    }

    while (m_u2TileIndex.y >= g_uSrtmSegment)
    {
        m_u2TileIndex.y -= g_uSrtmSegment;
        m_i2FileCoordinates.y++;
    }
}