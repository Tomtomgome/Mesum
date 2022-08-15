#include "Quaternion.hpp"

#include "MatGlm.hpp"

namespace m::math
{
mQuaternion::mQuaternion(mVec3 const& a_fromDirection,
                         mVec3 const& a_toDirection)
{
    glm::qua qua(M_VEC3_MESUM_TO_GLM(a_fromDirection),
                 M_VEC3_MESUM_TO_GLM(a_toDirection));
    *this = M_QUAT_GLM_TO_MESUM(qua);
}

mQuaternion::mQuaternion(mFloat a_x, mFloat a_y, mFloat a_z, mFloat a_w)
    : mVecData<float, 4>({a_x, a_y, a_z, a_w})
{
}


mVec3 operator*(mQuaternion const& a_q, mVec3 const& a_v)
{
    glm::vec3 result = M_QUAT_MESUM_TO_GLM(a_q) * M_VEC3_MESUM_TO_GLM(a_v);
    return M_VEC3_GLM_TO_MESUM(result);
}

}  // namespace m::math