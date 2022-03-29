#pragma once

#include "Serializable.hpp"
#include "Collision.hpp"

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
struct Key
{
    Serializable(1, Key);
    void display_gui();

    m::mFloat advancement = 0;
};

struct Modifier
{
    Serializable(1, Modifier);
    void display_gui();

    m::math::mVec4 color{};
    m::math::mVec2 offset{};
    m::mFloat      angle = 0;
    m::mFloat      scale = 0;
};

struct Animation
{
    Serializable(1, Animation);
    void display_gui();

    std::vector<Key>                    keys{{0.0f}, {1.0f}};
    std::vector<Modifier>               modifiers{{}, {}};
    std::chrono::steady_clock::duration animationDuration{};
    m::mUInt                            lastKeyIndex = 0;
    Modifier                            lastModifier{};
    m::mFloat                           currentAdvancement = 0;
    m::mBool                            isLooping          = true;
    m::mBool                            colorMultiply = true;  // Otherwise Add
    m::mBool                            scaleMultiply = true;  // Otherwise Add
};

struct AnimatorCpnt
{
    Serializable(1, AnimatorCpnt);
    void display_gui();

    static const m::mU32 version = 1;
    Animation*           pAnimation;
    m::mBool             enabled;
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

void apply_modifierToTC(TransformCpnt& a_tc, Modifier const& a_modifier,
                        bool a_scaleMultiply);
void apply_modifierToRC(RenderingCpnt& a_rc, Modifier const& a_modifier,
                        bool a_colorMultiply);

void process_renderableObjects(
    std::vector<TransformCpnt> const& a_transforms,
    std::vector<RenderingCpnt> const& a_renderingCpnts,
    DrawingData&                      a_outputDrawingData);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void process_animatedObjects(
    std::vector<AnimatorCpnt>&                 a_animators,
    std::chrono::steady_clock::duration const& a_deltaTime);

void apply_animationModifiers(
    std::vector<AnimatorCpnt> const&  a_animators,
    std::vector<TransformCpnt> const& a_transforms,
    std::vector<RenderingCpnt> const& a_renderingCpnts,
    std::vector<TransformCpnt>&       a_outTransforms,
    std::vector<RenderingCpnt>&       a_outRenderingCpnts);