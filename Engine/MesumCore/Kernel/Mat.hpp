#pragma once

#include "Vec.hpp"

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////
namespace m::math
{
///////////////////////////////////////////////////////////////////////////////
/// \brief Holds the data for a matrix
///////////////////////////////////////////////////////////////////////////////
template <typename t_T, mUInt t_N, mUInt t_M>
struct alignas(sizeof(t_T) * t_N * t_M) mMatData
{
    union
    {
        t_T            data[t_N][t_M];
        mVec<t_T, t_M> row[t_N];
    };
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Structure representing a generic matrix
///
/// \tparam t_T The type of the data in the vector
/// \tparam t_N The number of rows in the matrix
/// \tparam t_M The number of columns in the matrix
///////////////////////////////////////////////////////////////////////////////
template <typename t_T, mUInt t_N, mUInt t_M>
struct mMat : public mMatData<t_T, t_N, t_M>
{
    mMat<t_T, t_N, t_M>& operator=(const mMat<t_T, t_N, t_M>& a_data);

    void transpose();
};

template <typename t_T, mUInt t_N, mUInt t_M>
mMat<t_T, t_N, t_M> operator*(const mMat<t_T, t_N, t_M>& a_lhs,
                              const mMat<t_T, t_N, t_M>& a_rhs);

#define M_INTERNAL_FOR_EACH         \
    for (mUInt i = 0; i < t_N; ++i) \
        for (mUInt j = 0; j < t_M; ++j)

#define M_INTERNAL_FOR_EACH_ROW for (mUInt i = 0; i < t_N; ++i)

#define M_INTERNAL_FOR_EACH_COLUMN for (mUInt j = 0; j < t_M; ++j)

template <typename t_T, mUInt t_N, mUInt t_M>
mMat<t_T, t_N, t_M>& mMat<t_T, t_N, t_M>::operator=(
    const mMat<t_T, t_N, t_M>& a_data)
{
    std::memcpy(this, &a_data, sizeof(t_T) * t_N * t_M);
    return *this;
}

template <typename t_T, mUInt t_N, mUInt t_M>
void mMat<t_T, t_N, t_M>::transpose()
{
    for (mUInt i = 0; i < t_N; ++i)
    for (mUInt j = 0; j < i; ++j)
    {
        t_T tmp    = mMatData<t_T, t_N, t_M>::data[i][j];
        mMatData<t_T, t_N, t_M>::data[i][j] = mMatData<t_T, t_N, t_M>::data[j][i];
        mMatData<t_T, t_N, t_M>::data[j][i] = tmp;
    }
}

template <typename t_T, mUInt t_N, mUInt t_M>
mMat<t_T, t_N, t_M> operator*(const mMat<t_T, t_N, t_M>& a_lhs,
                              const mMat<t_T, t_N, t_M>& a_rhs)
{
    mMat<t_T, t_N, t_M> result{};
    M_INTERNAL_FOR_EACH
    {
        t_T tmpRes{};
        for (mUInt k = 0; k < t_M; ++k) { tmpRes += a_lhs.data[i][k] * a_rhs.data[k][j]; }
        result.data[i][j] = tmpRes;
    }

    return result;
}

};  // namespace m::math

///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////