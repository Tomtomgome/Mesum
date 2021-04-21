#ifndef M_Math
#define M_MATH
#pragma once

#include <Types.hpp>
#include <MathTypes.hpp>

namespace m
{
namespace math
{
template <typename T>
T lerp(T const& a_a1, T const& a_a2, Float a_ratio)
{
    return (1.0f - a_ratio) * a_a1 + a_ratio * a_a2;
}

class RandomGenerator
{
   public:
    void init(U64 a_seed);

    U64 get_nextU64();
    U32 get_nextU32();
    Double get_nextDouble();
    Float  get_nextFloat();

private:
    struct xoshiro256ssState
    {
        U64 s[4];
    };

    xoshiro256ssState m_state;
};

}
}  // namespace m

#endif //M_Math