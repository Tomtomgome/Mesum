#pragma once

#include "Serializable.hpp"
#include "Collision.hpp"
#include "Animation.hpp"

#include <Kernel/Math.hpp>

#include <vector>

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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
using Entity = m::mU32;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct ComponentManager
{
    static const m::mU32 s_version = 2U;

    void display_gui();

    void   initialize();
    void   reset();
    void   load_fromCopy(ComponentManager const& a_source);
    void   load_fromFile(std::string const& a_path);
    void   save_toFile(std::string const& a_path) const;
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
    template <>
    void enable_component<AnimatorCpnt>(Entity const&       a_entity,
                                        AnimatorCpnt const& a_component);
    template <>
    void enable_component<CollisionCpnt>(Entity const&        a_entity,
                                         CollisionCpnt const& a_component);

    m::mU32                    entityCount = 0;
    std::vector<RenderingCpnt> renderingCpnts;
    std::vector<AnimatorCpnt>  animators;
    std::vector<TransformCpnt> transforms;
    std::vector<CollisionCpnt> collisions;
};

template <typename t_Component>
void ComponentManager::enable_component(Entity const&      a_entity,
                                        t_Component const& a_component)
{
    mNotImplemented
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct Model
{
    Serializable(1, Model);
    void display_gui();

    std::string   name{"Unnamed"};
    m::mUInt      ID{0};
    RenderingCpnt renderingCpnt{};
    AnimatorCpnt  animator{};
    TransformCpnt transform{};
    CollisionCpnt collision{};
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct ModelBank
{
    void load();
    void save();

    void display_gui();
    void display_selecter(m::mInt& a_animationID);

    std::vector<Model> models;
};

extern ModelBank g_modelBank;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct DrawableData
{
    m::math::mVec4 color;
    m::math::mVec3 offset;
    m::mFloat      angle;
    m::mFloat      size;
};

struct DrawingData
{
    void                                   clean_drawables();
    std::vector<std::vector<DrawableData>> materialDrawables;
};

void process_renderableObjects(
    std::vector<TransformCpnt> const& a_transforms,
    std::vector<RenderingCpnt> const& a_renderingCpnts,
    DrawingData&                      a_outputDrawingData);