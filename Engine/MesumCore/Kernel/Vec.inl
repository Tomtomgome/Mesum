#include <Asserts.hpp>
#include <Vec.hpp>
#include <cmath>
#include <cstring>

namespace m::math
{
#define INTERNAL_VEC_FOR_EACH_MEMBER for (mUInt i = 0; i < t_Size; ++i)

template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size>::mVec() = default;

template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size>::mVec(const mVecData<t_T, t_Size>& a_data)
{
    *this = a_data;
}

template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size>::mVec(const mVecData<t_T, t_Size>&& a_data)
{
    *this = a_data;
}

template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size>& mVec<t_T, t_Size>::operator=(
    const mVecData<t_T, t_Size>& a_data)
{
    std::memcpy(this, &a_data, sizeof(t_T) * t_Size);
    return *this;
}

template <typename t_T, mUInt t_Size>
t_T& mVec<t_T, t_Size>::operator[](const mUInt a_index)
{
    mSoftAssert(a_index < t_Size);
    return mVecData<t_T, t_Size>::data[a_index];
}

template <typename t_T, mUInt t_Size>
t_T mVec<t_T, t_Size>::operator[](const mUInt a_index) const
{
    mSoftAssert(a_index < t_Size);
    return mVecData<t_T, t_Size>::data[a_index];
}

template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size>& mVec<t_T, t_Size>::operator++()
{
    INTERNAL_VEC_FOR_EACH_MEMBER
    ++mVecData<t_T, t_Size>::data[i];
    return *this;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> mVec<t_T, t_Size>::operator++(int)
{
    mVec<t_T, t_Size> copy = *this;
                      operator++();
    return copy;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size>& mVec<t_T, t_Size>::operator--()
{
    INTERNAL_VEC_FOR_EACH_MEMBER
    ++mVecData<t_T, t_Size>::data[i];
    return *this;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> mVec<t_T, t_Size>::operator--(int)
{
    mVec<t_T, t_Size> copy = *this;
                      operator--();
    return copy;
}

template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size>& mVec<t_T, t_Size>::operator+=(const mVec<t_T, t_Size>& a_v)
{
    INTERNAL_VEC_FOR_EACH_MEMBER
    mVecData<t_T, t_Size>::data[i] += a_v.data[i];
    return *this;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size>& mVec<t_T, t_Size>::operator+=(const t_T a_t)
{
    INTERNAL_VEC_FOR_EACH_MEMBER
    mVecData<t_T, t_Size>::data[i] += a_t;
    return *this;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size>& mVec<t_T, t_Size>::operator-=(const mVec<t_T, t_Size>& a_v)
{
    INTERNAL_VEC_FOR_EACH_MEMBER
    mVecData<t_T, t_Size>::data[i] -= a_v.data[i];
    return *this;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size>& mVec<t_T, t_Size>::operator-=(const t_T a_t)
{
    INTERNAL_VEC_FOR_EACH_MEMBER
    mVecData<t_T, t_Size>::data[i] -= a_t;
    return *this;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size>& mVec<t_T, t_Size>::operator*=(const mVec<t_T, t_Size>& a_v)
{
    INTERNAL_VEC_FOR_EACH_MEMBER
    mVecData<t_T, t_Size>::data[i] *= a_v.data[i];
    return *this;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size>& mVec<t_T, t_Size>::operator*=(const t_T a_t)
{
    INTERNAL_VEC_FOR_EACH_MEMBER
    mVecData<t_T, t_Size>::data[i] *= a_t;
    return *this;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size>& mVec<t_T, t_Size>::operator/=(const mVec<t_T, t_Size>& a_v)
{
    INTERNAL_VEC_FOR_EACH_MEMBER
    mVecData<t_T, t_Size>::data[i] /= a_v.data[i];
    return *this;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size>& mVec<t_T, t_Size>::operator/=(const t_T a_t)
{
    INTERNAL_VEC_FOR_EACH_MEMBER
    mVecData<t_T, t_Size>::data[i] /= a_t;
    return *this;
}

template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> mVec<t_T, t_Size>::operator+() const
{
    return *this;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> mVec<t_T, t_Size>::operator-() const
{
    mVec<t_T, t_Size> res;
    INTERNAL_VEC_FOR_EACH_MEMBER
    res.Data[i] = -mVecData<t_T, t_Size>::data[i];
    return res;
}

template <typename t_T, mUInt t_Size>
mBool mVec<t_T, t_Size>::operator==(const mVec<t_T, t_Size>& a_v) const
{
    INTERNAL_VEC_FOR_EACH_MEMBER
    if (mVecData<t_T, t_Size>::data[i] != a_v.data[i])
        return false;
    return true;
}

template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator+(const mVec<t_T, t_Size>& a_lhs,
                            const mVec<t_T, t_Size>& a_rhs)
{
    return mVec<t_T, t_Size>(a_lhs) += a_rhs;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator-(const mVec<t_T, t_Size>& a_lhs,
                            const mVec<t_T, t_Size>& a_rhs)
{
    return mVec<t_T, t_Size>(a_lhs) -= a_rhs;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator*(const mVec<t_T, t_Size>& a_lhs,
                            const mVec<t_T, t_Size>& a_rhs)
{
    return mVec<t_T, t_Size>(a_lhs) *= a_rhs;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator/(const mVec<t_T, t_Size>& a_lhs,
                            const mVec<t_T, t_Size>& a_rhs)
{
    return mVec<t_T, t_Size>(a_lhs) /= a_rhs;
}

template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator+(const mVec<t_T, t_Size>& a_lhs, const t_T a_rhs)
{
    return mVec<t_T, t_Size>(a_lhs) += a_rhs;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator-(const mVec<t_T, t_Size>& a_lhs, const t_T a_rhs)
{
    return mVec<t_T, t_Size>(a_lhs) -= a_rhs;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator*(const mVec<t_T, t_Size>& a_lhs, const t_T a_rhs)
{
    return mVec<t_T, t_Size>(a_lhs) *= a_rhs;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator/(const mVec<t_T, t_Size>& a_lhs, const t_T a_rhs)
{
    return mVec<t_T, t_Size>(a_lhs) /= a_rhs;
}

template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator+(const t_T a_lhs, const mVec<t_T, t_Size>& a_rhs)
{
    return mVec<t_T, t_Size>(a_rhs) += a_lhs;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator-(const t_T a_lhs, const mVec<t_T, t_Size>& a_rhs)
{
    return mVec<t_T, t_Size>(a_rhs) -= a_lhs;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator*(const t_T a_lhs, const mVec<t_T, t_Size>& a_rhs)
{
    return mVec<t_T, t_Size>(a_rhs) *= a_lhs;
}
template <typename t_T, mUInt t_Size>
mVec<t_T, t_Size> operator/(const t_T a_lhs, const mVec<t_T, t_Size>& a_rhs)
{
    return mVec<t_T, t_Size>(a_rhs) /= a_lhs;
}

template <typename t_T, mUInt t_Size>
[[nodiscard]] mVec<t_T, t_Size> abs(const mVec<t_T, t_Size>& a_v)
{
    mVec<t_T, t_Size> res;
    INTERNAL_VEC_FOR_EACH_MEMBER
    res.data[i] = std::abs(a_v.data[i]);
    return res;
}
template <typename t_T, mUInt t_Size>
[[nodiscard]] t_T dot(const mVec<t_T, t_Size>& a_lhs,
                      const mVec<t_T, t_Size>& a_rhs)
{
    t_T res = {};
    INTERNAL_VEC_FOR_EACH_MEMBER
    res += a_lhs.data[i] * a_rhs.data[i];
    return res;
}
template <typename t_T, mUInt t_Size>
[[nodiscard]] t_T length(const mVec<t_T, t_Size>& a_v)
{
    return std::sqrt(sqlength(a_v));
}
template <typename t_T, mUInt t_Size>
[[nodiscard]] t_T sqlength(const mVec<t_T, t_Size>& a_v)
{
    return dot(a_v, a_v);
}
template <typename t_T, mUInt t_Size>
[[nodiscard]] mVec<t_T, t_Size> normalized(const mVec<t_T, t_Size>& a_v)
{
    return a_v / length(a_v);
}

#undef INTERNAL_VEC_FOR_EACH_MEMBER
}  // namespace m::math