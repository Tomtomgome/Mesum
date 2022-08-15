#pragma once

#include "../../../Externals/glm/glm.hpp"
#include "../../../Externals/glm/ext.hpp"

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Core
/// \{
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// \brief Namespace for math related utils
///////////////////////////////////////////////////////////////////////////////
// namespace m::math
//{
#define M_VEC3_MESUM_TO_GLM(mesumVec3) \
    glm::vec3((mesumVec3).x, (mesumVec3).y, (mesumVec3).z)
#define M_VEC3_GLM_TO_MESUM(glmVec3)          \
    mVec3                                     \
    {                                         \
        (glmVec3).x, (glmVec3).y, (glmVec3).z \
    }

#define M_QUAT_MESUM_TO_GLM(mesumQuat) \
    glm::qua((mesumQuat).w, (mesumQuat).x, (mesumQuat).y, (mesumQuat).z)
#define M_QUAT_GLM_TO_MESUM(glmQuat)                       \
    mQuaternion                                            \
    {                                                      \
        (glmQuat).x, (glmQuat).y, (glmQuat).z, (glmQuat).w \
    }
/*
#define M_MAT4_MESUM_TO_GLM(mesumMat4)                                         \
    glm::mat4({mesumMat4.data[0][0], mesumMat4.data[1][0], mesumMat4.data[2][0],
mesumMat4.data[3][0],                                          \
               mesumMat4.data[0][1], mesumMat4.data[1][1], mesumMat4.data[2][1],
mesumMat4.data[3][1],                                          \
               mesumMat4.data[0][2], mesumMat4.data[1][2], mesumMat4.data[2][2],
mesumMat4.data[3][2],                                          \
               mesumMat4.data[0][3], mesumMat4.data[1][3], mesumMat4.data[2][3],
mesumMat4.data[3][3]})

#define M_MAT4_GLM_TO_MESUM(glmMat4)                               \
    mMat4x4{glmMat4[0][0], glmMat4[1][0], glmMat4[2][0], glmMat4[3][0], \
           glmMat4[0][1], glmMat4[1][1], glmMat4[2][1], glmMat4[3][1], \
           glmMat4[0][2], glmMat4[1][2], glmMat4[2][2], glmMat4[3][2], \
           glmMat4[0][3], glmMat4[1][3], glmMat4[2][3], glmMat4[3][3]}
*/

#define M_MAT4_MESUM_TO_GLM(mesumMat4)                                     \
    glm::mat4(                                                             \
        {mesumMat4.data[0][0], mesumMat4.data[0][1], mesumMat4.data[0][2], \
         mesumMat4.data[0][3], mesumMat4.data[1][0], mesumMat4.data[1][1], \
         mesumMat4.data[1][2], mesumMat4.data[1][3], mesumMat4.data[2][0], \
         mesumMat4.data[2][1], mesumMat4.data[2][2], mesumMat4.data[2][3], \
         mesumMat4.data[3][0], mesumMat4.data[3][1], mesumMat4.data[3][2], \
         mesumMat4.data[3][3]})

#define M_MAT4_GLM_TO_MESUM(glmMat4)                                    \
    mMat4x4                                                             \
    {                                                                   \
        glmMat4[0][0], glmMat4[0][1], glmMat4[0][2], glmMat4[0][3],     \
            glmMat4[1][0], glmMat4[1][1], glmMat4[1][2], glmMat4[1][3], \
            glmMat4[2][0], glmMat4[2][1], glmMat4[2][2], glmMat4[2][3], \
            glmMat4[3][0], glmMat4[3][1], glmMat4[3][2], glmMat4[3][3]  \
    }

//};  // namespace m::math

///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////