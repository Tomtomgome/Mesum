#ifndef M_MATHTYPES
#define M_MATHTYPES
#pragma once

#include "Vec.hpp"

#include "emmintrin.h"
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

        Vec4 simd_add(Vec4& a_v1, Vec4& a_v2)
        {
            Vec4 res;
            __m128* simd_data_v1 = (__m128*)a_v1.data;
            __m128* simd_data_v2 = (__m128*)a_v2.data;
            __m128* simd_data_res = (__m128*)res.data;

            *simd_data_res = _mm_add_ps(*simd_data_v1, *simd_data_v2);

            return res;
        }
    };
};

#endif //M_MATHTYPES