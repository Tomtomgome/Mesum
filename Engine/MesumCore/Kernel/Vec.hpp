#ifndef VEC_H
#define VEC_H

#include <Types.hpp>

namespace m
{
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
    namespace math
    {
        template<typename T, UInt Size>
        struct VecData
        {
            T data[Size];
        };

        template<typename T>
        struct VecData<T, 2>
        {
            union {
                T data[2];
                struct
                {
                    T x;
                    T y;
                };
            };
        };

        template<typename T>
        struct VecData<T, 3>
        {
            union {
                T data[3];
                struct
                {
                    T x;
                    T y;
                    T z;
                };
            };
        };

        template<typename T>
        struct VecData<T, 4>
        {   
            union
            {
                T data[4];
                struct
                {
                    T x;
                    T y;
                    T z;
                    T w;
                };
            }; 
        };

        template<typename T, UInt Size>
        struct Vec : public VecData<T, Size>
        {
            Vec();
            Vec(const VecData<T, Size>& a_data);

            Vec<T, Size>& operator=(const VecData<T, Size>& a_data);

            T& operator[](const UInt a_index);
            T operator[](const UInt a_index) const;

            Vec<T, Size>& operator++();
            Vec<T, Size> operator++(int);
            Vec<T, Size>& operator--();
            Vec<T, Size> operator--(int);

            Vec<T, Size>& operator+=(const Vec<T, Size>& a_v);
            Vec<T, Size>& operator+=(const T a_t);
            Vec<T, Size>& operator-=(const Vec<T, Size>& a_v);
            Vec<T, Size>& operator-=(const T a_t);
            Vec<T, Size>& operator*=(const Vec<T, Size>& a_v);
            Vec<T, Size>& operator*=(const T a_t);
            Vec<T, Size>& operator/=(const Vec<T, Size>& a_v);
            Vec<T, Size>& operator/=(const T a_t);

            Vec<T, Size> operator+() const;
            Vec<T, Size> operator-() const;

            Bool operator==(const Vec<T, Size>& a_v) const;
        };

        template<typename T, UInt Size>
        Vec<T, Size> operator+(const Vec<T, Size>& a_lhs, const Vec<T, Size>& a_rhs);
        template<typename T, UInt Size>
        Vec<T, Size> operator-(const Vec<T, Size>& a_lhs, const Vec<T, Size>& a_rhs);
        template<typename T, UInt Size>
        Vec<T, Size> operator*(const Vec<T, Size>& a_lhs, const Vec<T, Size>& a_rhs);
        template<typename T, UInt Size>
        Vec<T, Size> operator/(const Vec<T, Size>& a_lhs, const Vec<T, Size>& a_rhs);

        template<typename T, UInt Size>
        Vec<T, Size> operator+(const Vec<T, Size>& a_lhs, const T a_rhs);
        template<typename T, UInt Size>
        Vec<T, Size> operator-(const Vec<T, Size>& a_lhs, const T a_rhs);
        template<typename T, UInt Size>
        Vec<T, Size> operator*(const Vec<T, Size>& a_lhs, const T a_rhs);
        template<typename T, UInt Size>
        Vec<T, Size> operator/(const Vec<T, Size>& a_lhs, const T a_rhs);

        template<typename T, UInt Size>
        Vec<T, Size> operator+(const T a_lhs, const Vec<T, Size>& a_rhs);
        template<typename T, UInt Size>
        Vec<T, Size> operator-(const T a_lhs, const Vec<T, Size>& a_rhs);
        template<typename T, UInt Size>
        Vec<T, Size> operator*(const T a_lhs, const Vec<T, Size>& a_rhs);
        template<typename T, UInt Size>
        Vec<T, Size> operator/(const T a_lhs, const Vec<T, Size>& a_rhs);

        
        template<typename T, UInt Size>
        [[nodiscard]] Vec<T, Size> abs(const Vec<T, Size>& a_v);
        template<typename T, UInt Size>
        [[nodiscard]] T dot(const Vec<T, Size>& a_lhs, const Vec<T, Size>& a_rhs);
        template<typename T, UInt Size>
        [[nodiscard]] T length(const Vec<T, Size>& a_v);
        template<typename T, UInt Size>
        [[nodiscard]] T sqlength(const Vec<T, Size>& a_v);
        template<typename T, UInt Size>
        [[nodiscard]] Vec<T, Size> normalized(const Vec<T, Size>& a_v);

    };
};

#include <Vec.inl>

#endif //VEC_H