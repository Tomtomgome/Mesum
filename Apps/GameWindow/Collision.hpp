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

m::mBool collision_point(CollisionData const&  a_collisionData,
                     m::math::mVec2 const& a_point);

m::mBool collision_shape(CollisionData const&  a_cdA,
                     CollisionData const&  a_cdB);

void gather_intersectedObjects(
    std::vector<CollisionData> const& a_collisionDatas,
    m::math::mVec2 const& a_point, std::vector<Entity>& a_intersectedEntities);

void gather_intersectedObjects(
    std::vector<CollisionData> const&       a_collisionDatas,
    std::vector<std::pair<Entity, Entity>>& a_intersectedEntities);

m::mUInt draw_debugCollisions(
    std::vector<CollisionData> const& a_collisionDatas,
    m::render::DataMeshBuffer<m::render::BasicVertex, m::mU16>& a_meshBuffer);
