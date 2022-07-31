#pragma once

#include "Serializable.hpp"
#include <Kernel/Math.hpp>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct RenderingCpnt
{
    Serializable(1, RenderingCpnt);
    void display_gui();

    m::math::mVec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    m::mU32        materialID{0};
    m::mU32        pictureSize{0};
    m::mBool       enabled{false};
};
mBegin_Serialization(RenderingCpnt, 1)

    mSerialize_from(1, a_object.color.r);
mSerialize_from(1, a_object.color.g);
mSerialize_from(1, a_object.color.b);
mSerialize_from(1, a_object.color.a);
mSerialize_from(1, a_object.materialID);
mSerialize_from(1, a_object.pictureSize);
mSerialize_from(1, a_object.enabled);

mEnd_serialization(RenderingCpnt);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct TransformCpnt
{
    Serializable(1, TransformCpnt);
    void display_gui();

    m::math::mVec2 position{0.0f, 0.0f};
    m::mFloat      angle{0};
    m::mFloat      scale{1.0f};
    m::mBool       enabled{true};
};
mBegin_Serialization(TransformCpnt, 1)

    mSerialize_from(1, position.x);
mSerialize_from(1, position.y);
mSerialize_from(1, angle);
mSerialize_from(1, scale);
mSerialize_from(1, enabled);

mEnd_serialization(TransformCpnt);

TransformCpnt apply_transformToTC(TransformCpnt const& a_transformA,
                                  TransformCpnt const& a_transformB);