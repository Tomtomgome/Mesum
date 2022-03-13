#pragma once

#include <Kernel/Math.hpp>

#include <vector>

struct RenderingCpnt
{
    m::mU32  materialID;
    m::mU32  pictureSize;
    m::mBool enabled;
};

struct TransformCpnt
{
    m::math::mVec2 position;
    m::mFloat      angle;
    m::mFloat      scale;
    m::mBool       enabled;
};

using Entity = m::mU32;

struct ComponentManager
{
    void   initialize();
    Entity create_entity();

    template <typename t_Component>
    void enable_component(Entity const&      a_entity,
                          t_Component const& a_component);
    template <>
    void enable_component<RenderingCpnt>(Entity const&        a_entity,
                                         RenderingCpnt const& a_component);
    template <>
    void enable_component<TransformCpnt>(Entity const&        a_entity,
                                         TransformCpnt const& a_component);

    m::mU32 entityCount = 0;
    std::vector<RenderingCpnt> renderingCpnts;
    std::vector<TransformCpnt> transforms;
};

template <typename t_Component>
void ComponentManager::enable_component(Entity const&      a_entity,
                                        t_Component const& a_component)
{
    mNotImplemented
}

struct DrawableData
{
    m::math::mVec4 color;
    m::math::mVec3 offset;
    m::mFloat      angle;
    m::mFloat      size;
};

struct DrawingData
{
    void clean_drawables();
    std::vector<std::vector<DrawableData>> materialDrawables;
};

void process_renderableObjects(ComponentManager const& a_cpntManager,
                               DrawingData& a_outputDrawingData);

