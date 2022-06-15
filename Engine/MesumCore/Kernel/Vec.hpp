#pragma once

#include "Types.hpp"
#include <initializer_list>

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////
namespace m::math
{
///////////////////////////////////////////////////////////////////////////////
/// \brief Holds the data for a math vector
///////////////////////////////////////////////////////////////////////////////
template <typename t_T, mUInt t_Size>
struct mVecData
{
    t_T data[t_Size];
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Alias the data for a 2D math vector
///////////////////////////////////////////////////////////////////////////////
template <typename t_T>
struct mVecData<t_T, 2>
{
    union
    {
        t_T data[2] = {};
        struct
        {
            t_T x;
            t_T y;
        };
    };
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Alias the data for a 3D math vector
///////////////////////////////////////////////////////////////////////////////
template <typename t_T>
struct mVecData<t_T, 3>
{
    union
    {
        t_T data[3] = {};
        struct
        {
            t_T x;
            t_T y;
            t_T z;
        };
    };
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Alias the data for a 4D math vector
///////////////////////////////////////////////////////////////////////////////
template <typename t_T>
struct mVecData<t_T, 4>
{
    union
    {
        t_T data[4] = {};
        struct
        {
            t_T x;
            t_T y;
            t_T z;
            t_T w;
        };
        struct
        {
            t_T r;
            t_T g;
            t_T b;
            t_T a;
        };
    };
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Structure representing a generic math vector
///
/// \tparam t_T The type of the data in the vector
/// \tparam t_Size The dimension pf the vector
///////////////////////////////////////////////////////////////////////////////
template <typename t_T, mUInt t_Size>
struct mVec : public mVecData<t_T, t_Size>
{
    mVec();
    mVec(const std::initializer_list<t_T>& a_list);
    mVec(const mVecData<t_T, t_Size>& a_data);
    mVec(const mVecData<t_T, t_Size>&& a_data);

    mVec<t_T, t_Size>& operator=(const mVecData<t_T, t_Size>& a_data);

    t_T& operator[](mUInt a_index);
    t_T  operator[](mUInt a_index) const;

    mVec<t_T, t_Size>& operator++();
    mVec<t_T, t_Size>  operator++(int);
    mVec<t_T, t_Size>& operator--();
    mVec<t_T, t_Size>  operator--(int);

    mVec<t_T, t_Size>& operator+=(const mVec<t_T, t_Size>& a_v);
    mVec<t_T, t_Size>& operator+=(t_T a_t);
    mVec<t_T, t_Size>& operator-=(const mVec<t_T, t_Size>& a_v);
    mVec<t_T, t_Size>& operator-=(t_T a_t);
    mVec<t_T, t_Size>& operator*=(const mVec<t_T, t_Size>& a_v);
    mVec<t_T, t_Size>& operator*=(t_T a_t);
    mVec<t_T, t_Size>& operator/=(const mVec<t_T, t_Size>& a_v);
    mVec<t_T, t_Size>& operator/=(t_T a_t);

    mVec<t_T, t_Size> operator+() const;
    mVec<t_T, t_Size> operator-() const;

    mBool operator==(const mVec<t_T, t_Size>& a_v) const;
    mBool operator<(const mVec<t_T, t_Size>& a_v) const;
};

template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator+(const mVec<t_T, t_Size>& a_lhs,
                            const mVec<t_T, t_Size>& a_rhs);
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator-(const mVec<t_T, t_Size>& a_lhs,
                            const mVec<t_T, t_Size>& a_rhs);
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator*(const mVec<t_T, t_Size>& a_lhs,
                            const mVec<t_T, t_Size>& a_rhs);
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator/(const mVec<t_T, t_Size>& a_lhs,
                            const mVec<t_T, t_Size>& a_rhs);

template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator+(const mVec<t_T, t_Size>& a_lhs, t_T a_rhs);
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator-(const mVec<t_T, t_Size>& a_lhs, t_T a_rhs);
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator*(const mVec<t_T, t_Size>& a_lhs, t_T a_rhs);
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator/(const mVec<t_T, t_Size>& a_lhs, t_T a_rhs);

template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator+(t_T a_lhs, const mVec<t_T, t_Size>& a_rhs);
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator-(t_T a_lhs, const mVec<t_T, t_Size>& a_rhs);
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator*(t_T a_lhs, const mVec<t_T, t_Size>& a_rhs);
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator/(t_T a_lhs, const mVec<t_T, t_Size>& a_rhs);

template <typename t_T, mUInt t_Size>
[[nodiscard]] mVec<t_T, t_Size> abs(const mVec<t_T, t_Size>& a_v);
template <typename t_T, mUInt t_Size>
[[nodiscard]] t_T dot(const mVec<t_T, t_Size>& a_lhs,
                      const mVec<t_T, t_Size>& a_rhs);
template <typename t_T, mUInt t_Size>
[[nodiscard]] t_T length(const mVec<t_T, t_Size>& a_v);
template <typename t_T, mUInt t_Size>
[[nodiscard]] t_T sqlength(const mVec<t_T, t_Size>& a_v);
template <typename t_T, mUInt t_Size>
[[nodiscard]] mVec<t_T, t_Size> normalized(const mVec<t_T, t_Size>& a_v);

};  // namespace m::math

#include "Vec.inl"

///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////