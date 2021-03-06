#ifndef M_MATHTYPES
#define M_MATHTYPES
#pragma once

#include <Vec.hpp>
#include <cstring>

namespace m {
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
    namespace math
    {
        using Vec2 = Vec<Float, 2>;
        using Vec3 = Vec<Float, 3>;
        using Vec4 = Vec<Float, 4>;

        using DVec2 = Vec<Double, 2>;
        using DVec3 = Vec<Double, 3>;
        using DVec4 = Vec<Double, 4>;

        using IVec2 = Vec<Int, 2>;
        using IVec3 = Vec<Int, 3>;
        using IVec4 = Vec<Int, 4>;

        using UIVec2 = Vec<UInt, 2>;
        using UIVec3 = Vec<UInt, 3>;
        using UIVec4 = Vec<UInt, 4>;
    };
};

#endif //M_MATHTYPES