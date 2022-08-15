#pragma once

#include <Kernel/Serializable.hpp>
#include <Kernel/Math.hpp>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct RenderingCpnt
{
    void display_gui();

    m::math::mVec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    m::mU32        materialID{0};
    m::mU32        pictureSize{0};
    m::mBool       enabled{false};
};
mBegin_serialization(RenderingCpnt, 1);

mSerialize_memberFrom(1, color.r);
mSerialize_memberFrom(1, color.g);
mSerialize_memberFrom(1, color.b);
mSerialize_memberFrom(1, color.a);
mSerialize_memberFrom(1, materialID);
mSerialize_memberFrom(1, pictureSize);
mSerialize_memberFrom(1, enabled);

mEnd_serialization(RenderingCpnt);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct TransformCpnt
{
    void display_gui();

    m::math::mVec2 position{0.0f, 0.0f};
    m::mFloat      angle{0};
    m::mFloat      scale{1.0f};
    m::mBool       enabled{true};
};
mBegin_serialization(TransformCpnt, 1);

mSerialize_memberFrom(1, position.x);
mSerialize_memberFrom(1, position.y);
mSerialize_memberFrom(1, angle);
mSerialize_memberFrom(1, scale);
mSerialize_memberFrom(1, enabled);

mEnd_serialization(TransformCpnt);

TransformCpnt apply_transformToTC(TransformCpnt const& a_transformA,
                                  TransformCpnt const& a_transformB);