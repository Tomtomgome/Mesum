#include "Math/MathTypes.hpp"
#include "Asserts.hpp"
#include "Logger.hpp"

#include <iostream>
#include <ctime>
#include <chrono>

struct Profiling
{
    Profiling(m::Double* const a_resultHandle)
    {
        handle = a_resultHandle;
        start = std::chrono::high_resolution_clock::now();
    }
    ~Profiling()
    {
        auto end = std::chrono::high_resolution_clock::now();
        *handle = std::chrono::duration_cast< std::chrono::milliseconds >( end-start ).count();
    }

    std::chrono::high_resolution_clock::time_point start;
    m::Double* handle;
};

void initialize_data(m::math::Vec4* a_vecs1, m::math::Vec4* a_vecs2, m::UInt a_nbElements)
{
    for(m::UInt i = 0; i < a_nbElements; ++i)
    {
        a_vecs1[i].x = rand();
        a_vecs1[i].y = rand();
        a_vecs1[i].z = rand();
        a_vecs1[i].w = rand();

        a_vecs2[i].x = -rand();
        a_vecs2[i].y = -rand();
        a_vecs2[i].z = -rand();
        a_vecs2[i].w = -rand();
    }
}

struct SimpleVec4
{
    int x;
    int y;
    int z;
    int w;
};

SimpleVec4 add(const SimpleVec4& a_v1, const SimpleVec4& a_v2)
{
    return {a_v1.x + a_v2.x, a_v1.y + a_v2.y, a_v1.z + a_v2.z, a_v1.w + a_v2.w};
}

m::Int main(m::Int argc, char *argv[])
{
    /*
    mHardAssert(true);
    m::math::Vec<m::Float, 4> vec1;
    vec1[0] = 1.0F;
    vec1[1] = 2.0F;
    vec1[2] = 3.0F;
    vec1[3] = 4.0F;

    m::math::Vec<m::Float, 4> vec2;
    vec2[0] = 1.0F;
    vec2[1] = 2.0F;
    vec2[2] = 3.0F;
    vec2[3] = 4.0F;

    vec1 = vec1 + vec2;
    vec1++;
    vec1*=2.0F;
    LOG("Le vecteur : ", vec1.x, " ", vec1.w);

    m::math::Vec4 vec3;
    vec3.x = 6.0F;
    vec3.y = 10.0F;
    vec3.z = 14.0F;
    vec3.w = 18.0F;

    if(vec1 == vec3)
        LOG("Success !");

    LOG(dot(vec1, vec3));
*/
    m::UInt nbElements = 100000; 
    m::UInt AddIterations = 100;
    if(argc > 1)
    {
        AddIterations = std::stoi(argv[1]);
    }

    m::Double timeSimpleAdd = 0.0;
    m::Double timeSimpleOnlyAdd = 0.0;
    m::Double timeSimdAdd = 0.0;
    m::Double timeSimdOnlyAdd = 0.0;
    m::Double timeArrayAdd = 0.0;
    m::Double timeArrayOnlyAdd = 0.0;
    m::Double timeSimdArrayAdd = 0.0;
    m::Double timeSimdArrayOnlyAdd = 0.0;

    m::Float result = 0.0F;
    {
        Profiling p(&timeSimpleAdd);
        m::math::Vec4 vecs1[nbElements];
        m::math::Vec4 vecs2[nbElements];
        m::math::Vec4 vec[nbElements];
        initialize_data(vecs1, vecs2, nbElements);

        {
            Profiling p2(&timeSimpleOnlyAdd);
            for(m::UInt j = 0; j<AddIterations; ++j)
            {
                for(m::UInt i = 0; i < nbElements; ++i)
                {
                    vec[i] = vecs1[i] + vecs2[i];
                }
            }
        }

        for(m::UInt i = 0; i < nbElements; ++i)
        {
            result += vec[i].x + vec[i].y + vec[i].z + vec[i].w;
        }
    }
    std::cout << "result : " << result << " in " << timeSimpleAdd << "ms(" << timeSimpleOnlyAdd << " just for adds)" << std::endl;






    m::Double timeTRISTE = 0.0;
    m::Double timeTRISTEadds = 0.0;
    result = 0.0F;
    {
        Profiling p(&timeTRISTE);
        SimpleVec4 vecs1[nbElements];
        SimpleVec4 vecs2[nbElements];
        SimpleVec4 vec[nbElements];
        for(m::UInt i = 0; i < nbElements; ++i)
        {
            vecs1[i].x = rand();
            vecs1[i].y = rand();
            vecs1[i].z = rand();
            vecs1[i].w = rand();

            vecs2[i].x = -rand();
            vecs2[i].y = -rand();
            vecs2[i].z = -rand();
            vecs2[i].w = -rand();
        }

        {
            Profiling p2(&timeTRISTEadds);
            for(m::UInt j = 0; j<AddIterations; ++j)
            {
                for(m::UInt i = 0; i < nbElements; ++i)
                {
                    vec[i] = add(vecs1[i], vecs2[i]);
                }
            }
        }

        for(m::UInt i = 0; i < nbElements; ++i)
        {
            result += vec[i].x + vec[i].y + vec[i].z + vec[i].w;
        }
    }
    std::cout << "result TRISTE : " << result << " in " << timeTRISTE << "ms(" << timeTRISTEadds << " just for adds)" << std::endl;







    result = 0.0F; 
    {
        Profiling p(&timeSimdAdd);
        m::math::Vec4 vecs1[nbElements];
        m::math::Vec4 vecs2[nbElements];
        m::math::Vec4 vec[nbElements];
        initialize_data(vecs1, vecs2, nbElements);

        {
            Profiling p2(&timeSimdOnlyAdd);
            for(m::UInt j = 0; j<AddIterations; ++j)
            {
                for(m::UInt i = 0; i < nbElements; ++i)
                {
                    vec[i] = simd_add(vecs1[i], vecs2[i]);
                }
            }
        }

        for(m::UInt i = 0; i < nbElements; ++i)
        {
            result += vec[i].x + vec[i].y + vec[i].z + vec[i].w;
        }
    }
    std::cout << "result Simd : " << result << " in " << timeSimdAdd << "ms(" << timeSimdOnlyAdd << " just for adds)" << std::endl;

    result = 0.0F;
    {
        Profiling p(&timeArrayAdd);

        
        m::math::Vec4 vecs1[nbElements];
        m::math::Vec4 vecs2[nbElements];
        m::math::Vec4 vec[nbElements];
        initialize_data(vecs1, vecs2, nbElements);
        
        {
            Profiling p(&timeArrayOnlyAdd);

            for(m::UInt j = 0; j<AddIterations; ++j)
            {
                float* fvecs1 = (float*)&vecs1;
                float* fvecs2 = (float*)&vecs2;
                float* fvecs = (float*)&vec;
                for(m::UInt i = 0; i < (nbElements-1)*4; ++i, ++fvecs, ++fvecs1, ++fvecs2)
                {
                    *fvecs = *fvecs1 + *fvecs2;  
                }
            }
        }

        for(m::UInt i = 0; i < nbElements; ++i)
        {
            result += vec[i].x + vec[i].y + vec[i].z + vec[i].w;
        }
    }
    std::cout << "result array : " << result << " in " << timeArrayAdd << "ms(" << timeArrayOnlyAdd << " just for adds)" << std::endl;

    result = 0.0F;
    {
        Profiling p(&timeSimdArrayAdd);

        m::math::Vec4 vecs1[nbElements];
        m::math::Vec4 vecs2[nbElements];
        m::math::Vec4 vec[nbElements];
        initialize_data(vecs1, vecs2, nbElements);
        
        {
            Profiling p(&timeSimdArrayOnlyAdd);
            for(m::UInt j = 0; j<AddIterations; ++j)
            {
                __m128* fvecs1 = (__m128*)&vecs1;
                __m128* fvecs2 = (__m128*)&vecs2;
                __m128* fvecs = (__m128*)&vec;

                for (m::UInt i = 0; i < nbElements; ++i, ++fvecs, ++fvecs1, ++fvecs2)
                    *fvecs = _mm_add_ps(*fvecs1, *fvecs2);
            }
        }

        for(m::UInt i = 0; i < nbElements; ++i)
        {
            result += vec[i].x + vec[i].y + vec[i].z + vec[i].w;
        }
    }

    std::cout << "result simd array : " << result << " in " << timeSimdArrayAdd << "ms(" << timeSimdArrayOnlyAdd << " just for adds)" << std::endl;

    return 0;
}