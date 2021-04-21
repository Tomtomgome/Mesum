#include <Math.hpp>

namespace m
{
namespace math
{
struct splitmix64State
{
    U64 s;
};

U64 splitmix64(splitmix64State* state)
{
    U64 result = state->s;

    state->s = result + 0x9E3779B97f4A7C15;
    result   = (result ^ (result >> 30)) * 0xBF58476D1CE4E5B9;
    result   = (result ^ (result >> 27)) * 0x94D049BB133111EB;
    return result ^ (result >> 31);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void RandomGenerator::init(U64 a_seed)
{
    splitmix64State  smstate = {a_seed};

    U64 tmp  = splitmix64(&smstate);
    m_state.s[0] = (U32)tmp;
    m_state.s[1] = (U32)(tmp >> 32);

    tmp      = splitmix64(&smstate);
    m_state.s[2] = (U32)tmp;
    m_state.s[3] = (U32)(tmp >> 32);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
U64 rol64(U64 a_x, Int a_k)
{
    return (a_x << a_k) | (a_x >> (64 - a_k));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
U64 RandomGenerator::get_nextU64()
{
    U64*      s      = m_state.s;
    U64 const result = rol64(s[1] * 5, 7) * 9;
    U64 const t      = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;
    s[3] = rol64(s[3], 45);

    return result;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
U32 RandomGenerator::get_nextU32()
{
    return U32(get_nextU64());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Double RandomGenerator::get_nextDouble()
{
    return (get_nextU64() >> 11) * 0x1.0p-53;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Float RandomGenerator::get_nextFloat()
{
    return Float(get_nextDouble());
}

}  // namespace math
}  // namespace m