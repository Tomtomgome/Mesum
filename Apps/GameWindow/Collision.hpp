#pragma once

#include <Kernel/Math.hpp>
#include <Kernel/Serializable.hpp>

#include <MesumGraphics/RenderBase.hpp>

#include <vector>

struct TransformCpnt;
using Entity = m::mU32;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct CollisionCpnt
{
    void display_gui();

    std::vector<m::math::mVec2> positions{{}, {}, {}};
    m::mBool                    enabled;
};
mBegin_serialization(CollisionCpnt, 1);

m::mUInt nbPositions = a_object.positions.size();
mSerialize_from(1, nbPositions);
a_object.positions.resize(nbPositions);
for (auto& position : a_object.positions)
{
    mSerialize_from(1, position.x);
    mSerialize_from(1, position.y);
}
mSerialize_memberFrom(1, enabled);

mEnd_serialization(CollisionCpnt);

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

m::mBool collision_shape(CollisionData const& a_cdA,
                         CollisionData const& a_cdB);

void gather_intersectedObjects(
    std::vector<CollisionData> const& a_collisionDatas,
    m::math::mVec2 const& a_point, std::vector<Entity>& a_intersectedEntities);

void gather_intersectedObjects(
    std::vector<CollisionData> const&       a_collisionDatas,
    std::vector<std::pair<Entity, Entity>>& a_intersectedEntities);

m::mUInt draw_debugCollisions(
    std::vector<CollisionData> const& a_collisionDatas,
    m::render::DataMeshBuffer<m::render::BasicVertex, m::mU16>& a_meshBuffer);
