#include "Vec.hpp"
#include <Kernel/Asserts.hpp>
#include <cstring>
#include <cmath>

namespace m::math
{
    #define INTERNAL_VEC_FOR_EACH_MEMBER  for(UInt i = 0; i < Size; ++i)

    template<typename T, UInt Size>
    Vec<T, Size>::Vec() = default;

    template<typename T, UInt Size>
    Vec<T, Size>::Vec(const VecData<T, Size>& a_data)
    {
        *this = a_data;
    }

    template<typename T, UInt Size>
    Vec<T, Size>& Vec<T, Size>::operator=(const VecData<T, Size>& a_data)
    {
        std::memcpy(this, &a_data, sizeof(T)*Size);
        return *this;
    }

    template<typename T, UInt Size>
    T& Vec<T, Size>::operator[](const UInt a_index){
        mAssert(a_index < Size);
        return VecData<T, Size>::data[a_index];
    }

    template<typename T, UInt Size>
    T Vec<T, Size>::operator[](const UInt a_index) const{
        mAssert(a_index < Size);
        return VecData<T, Size>::data[a_index];
    }



    template<typename T, UInt Size>
    Vec<T, Size>& Vec<T, Size>::operator++(){
        INTERNAL_VEC_FOR_EACH_MEMBER
            ++VecData<T,Size>::data[i];
        return *this;
    }
    template<typename T, UInt Size>
    Vec<T, Size> Vec<T, Size>::operator++(int){
        Vec<T, Size> copy = *this;
        operator++();
        return copy;
    }
    template<typename T, UInt Size>
    Vec<T, Size>& Vec<T, Size>::operator--(){
        INTERNAL_VEC_FOR_EACH_MEMBER
            ++VecData<T, Size>::data[i];
        return *this;
    }
    template<typename T, UInt Size>
    Vec<T, Size> Vec<T, Size>::operator--(int){
        Vec<T, Size> copy = *this;
        operator--();
        return copy;
    }
    
    template<typename T, UInt Size>
    Vec<T, Size>& Vec<T, Size>::operator+=(const Vec<T, Size>& a_v){
        INTERNAL_VEC_FOR_EACH_MEMBER
            VecData<T, Size>::data[i] += a_v.data[i];
        return *this;
    }
    template<typename T, UInt Size>
    Vec<T, Size>& Vec<T, Size>::operator+=(const T a_t){
        INTERNAL_VEC_FOR_EACH_MEMBER
            VecData<T, Size>::data[i] += a_t;
        return *this;
    }
    template<typename T, UInt Size>
    Vec<T, Size>& Vec<T, Size>::operator-=(const Vec<T, Size>& a_v){
        INTERNAL_VEC_FOR_EACH_MEMBER
            VecData<T, Size>::data[i] -= a_v.data[i];
        return *this;
    }
    template<typename T, UInt Size>
    Vec<T, Size>& Vec<T, Size>::operator-=(const T a_t){
        INTERNAL_VEC_FOR_EACH_MEMBER
            VecData<T, Size>::data[i] -= a_t;
        return *this;
    }
    template<typename T, UInt Size>
    Vec<T, Size>& Vec<T, Size>::operator*=(const Vec<T, Size>& a_v){
        INTERNAL_VEC_FOR_EACH_MEMBER
            VecData<T, Size>::data[i] *= a_v.data[i];
        return *this;
    }
    template<typename T, UInt Size>
    Vec<T, Size>& Vec<T, Size>::operator*=(const T a_t){
        INTERNAL_VEC_FOR_EACH_MEMBER
            VecData<T, Size>::data[i] *= a_t;
        return *this;
    }
    template<typename T, UInt Size>
    Vec<T, Size>& Vec<T, Size>::operator/=(const Vec<T, Size>& a_v){
        INTERNAL_VEC_FOR_EACH_MEMBER
            VecData<T, Size>::data[i] /= a_v.data[i];
        return *this;
    }
    template<typename T, UInt Size>
    Vec<T, Size>& Vec<T, Size>::operator/=(const T a_t){
        INTERNAL_VEC_FOR_EACH_MEMBER
            VecData<T, Size>::data[i] /= a_t;
        return *this;
    }

    template<typename T, UInt Size>
    Vec<T, Size> Vec<T, Size>::operator+() const{
        return *this;
    }
    template<typename T, UInt Size>
    Vec<T, Size> Vec<T, Size>::operator-() const{
        Vec<T, Size> res;
        INTERNAL_VEC_FOR_EACH_MEMBER
            res.Data[i] = -VecData<T, Size>::data[i];
        return res;
    }

    template<typename T, UInt Size>
    Bool Vec<T, Size>::operator==(const Vec<T, Size>& a_v) const{
        INTERNAL_VEC_FOR_EACH_MEMBER
            if(VecData<T, Size>::data[i] != a_v.data[i]) return false;
        return true;
    }





    
    template<typename T, UInt Size>
    Vec<T, Size> operator+(const Vec<T, Size>& a_lhs, const Vec<T, Size>& a_rhs){
        return Vec<T, Size>(a_lhs) += a_rhs;
    }
    template<typename T, UInt Size>
    Vec<T, Size> operator-(const Vec<T, Size>& a_lhs, const Vec<T, Size>& a_rhs){
        return Vec<T, Size>(a_lhs) -= a_rhs;
    }
    template<typename T, UInt Size>
    Vec<T, Size> operator*(const Vec<T, Size>& a_lhs, const Vec<T, Size>& a_rhs){
        return Vec<T, Size>(a_lhs) *= a_rhs;
    }
    template<typename T, UInt Size>
    Vec<T, Size> operator/(const Vec<T, Size>& a_lhs, const Vec<T, Size>& a_rhs){
        return Vec<T, Size>(a_lhs) /= a_rhs;
    }

    template<typename T, UInt Size>
    Vec<T, Size> operator+(const Vec<T, Size>& a_lhs, const T a_rhs){
        return Vec<T, Size>(a_lhs) += a_rhs;
    }
    template<typename T, UInt Size>
    Vec<T, Size> operator-(const Vec<T, Size>& a_lhs, const T a_rhs){
        return Vec<T, Size>(a_lhs) -= a_rhs;
    }
    template<typename T, UInt Size>
    Vec<T, Size> operator*(const Vec<T, Size>& a_lhs, const T a_rhs){
        return Vec<T, Size>(a_lhs) *= a_rhs;
    }
    template<typename T, UInt Size>
    Vec<T, Size> operator/(const Vec<T, Size>& a_lhs, const T a_rhs){
        return Vec<T, Size>(a_lhs) /= a_rhs;
    }

    template<typename T, UInt Size>
    Vec<T, Size> operator+(const T a_lhs, const Vec<T, Size>& a_rhs){
        return Vec<T, Size>(a_rhs) += a_lhs;
    }
    template<typename T, UInt Size>
    Vec<T, Size> operator-(const T a_lhs, const Vec<T, Size>& a_rhs){
        return Vec<T, Size>(a_rhs) -= a_lhs;
    }
    template<typename T, UInt Size>
    Vec<T, Size> operator*(const T a_lhs, const Vec<T, Size>& a_rhs){
        return Vec<T, Size>(a_rhs) *= a_lhs;
    }
    template<typename T, UInt Size>
    Vec<T, Size> operator/(const T a_lhs, const Vec<T, Size>& a_rhs){
        return Vec<T, Size>(a_rhs) /= a_lhs;
    }


    template<typename T, UInt Size>
    [[nodiscard]] Vec<T, Size> abs(const Vec<T, Size>& a_v)
    {
        Vec<T, Size> res;
        INTERNAL_VEC_FOR_EACH_MEMBER
            res.data[i] = std::abs(a_v.data[i]);
        return res;
    }
    template<typename T, UInt Size>
    [[nodiscard]] T dot(const Vec<T, Size>& a_lhs, const Vec<T, Size>& a_rhs)
    {
        T res = {};
        INTERNAL_VEC_FOR_EACH_MEMBER
            res += a_lhs.data[i]*a_rhs.data[i];
        return res;
    }
    template<typename T, UInt Size>
    [[nodiscard]] T length(const Vec<T, Size>& a_v)
    {
        return std::sqrt(sqlength(a_v));
    }
    template<typename T, UInt Size>
    [[nodiscard]] T sqlength(const Vec<T, Size>& a_v)
    {
        return dot(a_v, a_v);
    }
    template<typename T, UInt Size>
    [[nodiscard]] Vec<T, Size> normalized(const Vec<T, Size>& a_v)
    {
        return a_v / length(a_v);
    }


    #undef INTERNAL_VEC_FOR_EACH_MEMBER
}