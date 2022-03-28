#pragma once

#include "Serializable.hpp"
#include <Kernel/Math.hpp>

#include <MesumGraphics/RenderBase.hpp>

#include <vector>

class ComponentManager;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct CollisionCpnt
{
    Serializable(1, CollisionCpnt);
    void display_gui();

    std::vector<m::math::mVec2> positions{{}, {}, {}};
    m::mBool                    enabled;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
m::mUInt draw_debugCollisions(
    ComponentManager const&                                     a_cpntManager,
    m::render::DataMeshBuffer<m::render::BasicVertex, m::mU16>& a_meshBuffer);