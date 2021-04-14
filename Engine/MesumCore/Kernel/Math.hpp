#ifndef M_Math
#define M_MATH
#pragma once

#include <Types.hpp>

namespace m
{
namespace math
{
template <typename T>
T lerp(T const& a_a1, T const& a_a2, Float a_ratio)
{
    return (1.0f - a_ratio) * a_a1 + a_ratio * a_a2;
}
}
}  // namespace m

#endif //M_Math