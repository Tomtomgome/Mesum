#pragma once

#include "Kernel/Types.hpp"
#include "Kernel/MathTypes.hpp"

using namespace m;

class mHgtPosition
{
   public:
    mHgtPosition() = default;
    mHgtPosition(math::mIVec2 a_ri2FileCoordinates, math::mUIVec2 a_ru2Tile);

    mBool operator==(mHgtPosition const& a_rVal) const;

    void SetFromAngle(math::mVec2 a_rv2Angle);
    void Move(mInt a_uMoveX, mInt a_uMoveY);

    [[nodiscard]] math::mIVec2 const& GetFileCoordinates() const
    {
        return m_i2FileCoordinates;
    }
    [[nodiscard]] math::mUIVec2 const& GetTileIndex() const
    {
        return m_u2TileIndex;
    }

   private:
    math::mIVec2  m_i2FileCoordinates;
    math::mUIVec2 m_u2TileIndex;
};
