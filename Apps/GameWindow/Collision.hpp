#pragma once

#include "Serializable.hpp"
#include <Kernel/Math.hpp>

#include <MesumGraphics/RenderBase.hpp>

#include <vector>

class TransformCpnt;
using Entity = m::mU32;

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

struct CollisionData
{
    Entity                      entity;
    std::vector<m::math::mVec2> positions{{}, {}, {}};
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void process_collisions(std::vector<TransformCpnt> const& a_transforms,
                        std::vector<CollisionCpnt> const& a_collisions,
                        std::vector<CollisionData>&       a_collisionDatas);

bool collision_point(CollisionCpnt const&  a_collisionCpnt,
                     m::math::mVec2 const& a_point);

void gather_intersectedObjects(
    std::vector<CollisionData> const& a_collisionDatas,
    m::math::mVec2 const& a_point, std::vector<Entity>& a_intersectedEntities);

m::mUInt draw_debugCollisions(
    std::vector<CollisionData> const& a_collisionDatas,
    m::render::DataMeshBuffer<m::render::BasicVertex, m::mU16>& a_meshBuffer);
