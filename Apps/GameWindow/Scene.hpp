#pragma once

#include <Kernel/Math.hpp>

#include <vector>
#include <iostream>

#define Serializable(a_versionNumber, t_ClassName)           \
    static const m::mU32 s_version = a_versionNumber;        \
    void                 read(std::ifstream& a_inputStream); \
    void                 write(std::ofstream& a_outputStream) const;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct RenderingCpnt
{
    Serializable(1, RenderingCpnt);

    m::math::mVec4 color;
    m::mU32        materialID;
    m::mU32        pictureSize;
    m::mBool       enabled;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct TransformCpnt
{
    Serializable(1, TransformCpnt);

    m::math::mVec2 position;
    m::mFloat      angle;
    m::mFloat      scale;
    m::mBool       enabled;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct Key
{
    Serializable(1, Key);

    m::mFloat advancement = 0;
};

struct Modifier
{
    Serializable(1, Modifier);

    m::math::mVec4 color{};
    m::math::mVec2 offset{};
    m::mFloat      angle = 0;
    m::mFloat      scale = 0;
};

struct Animation
{
    Serializable(1, Animation);

    std::vector<Key>                    keys;
    std::vector<Modifier>               modifiers;
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
    static const m::mU32 s_version = 1U;

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

    m::mU32                    entityCount = 0;
    std::vector<RenderingCpnt> renderingCpnts;
    std::vector<AnimatorCpnt>  animators;
    std::vector<TransformCpnt> transforms;
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

void process_renderableObjects(ComponentManager const& a_cpntManager,
                               DrawingData&            a_outputDrawingData);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void process_animatedObjects(
    ComponentManager&                          a_cpntManager,
    std::chrono::steady_clock::duration const& a_deltaTime);