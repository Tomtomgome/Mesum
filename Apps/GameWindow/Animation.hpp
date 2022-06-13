#pragma once

#include "Serializable.hpp"
#include <Kernel/Math.hpp>

#include <vector>

class RenderingCpnt;
class TransformCpnt;
struct GameAction;
struct ComponentManager;
using Entity = m::mU32;

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
    Serializable(2, Animation);
    void display_gui();

    std::string                         name{"unnamed"};
    m::mUInt                            ID{0};
    std::vector<Key>                    keys{{0.0f}, {1.0f}};
    std::vector<Modifier>               modifiers{{}, {}};
    std::vector<GameAction*>            gameActions{{nullptr}, {nullptr}};
    std::chrono::steady_clock::duration animationDuration{};
    m::mBool                            colorMultiply = true;  // Otherwise Add
    m::mBool                            scaleMultiply = true;  // Otherwise Add
};

struct AnimationBank
{
    void load();
    void save();

    void display_gui();
    void display_animationSelecter(m::mInt& a_animationID);

    std::vector<Animation> animations;
};

extern AnimationBank g_animationBank;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct AnimatorCpnt
{
    Serializable(3, AnimatorCpnt);
    void display_gui();

    m::mInt           animationID{-1};
    Modifier          lastModifier{};
    m::mInt           lastKeyIndex{0};
    m::mFloat         currentAdvancement{0};
    m::mBool          isLooping{true};
    ComponentManager* pParentManager{nullptr};
    m::mBool          enabled{false};
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void apply_modifierToTC(TransformCpnt& a_tc, Modifier const& a_modifier,
                        bool a_scaleMultiply);
void apply_modifierToRC(RenderingCpnt& a_rc, Modifier const& a_modifier,
                        bool a_colorMultiply);

void process_animatedObjects(
    std::vector<AnimatorCpnt>&                 a_animators,
    std::chrono::steady_clock::duration const& a_deltaTime);

void apply_animationModifiers(
    std::vector<AnimatorCpnt> const&  a_animators,
    std::vector<TransformCpnt> const& a_transforms,
    std::vector<RenderingCpnt> const& a_renderingCpnts,
    std::vector<TransformCpnt>&       a_outTransforms,
    std::vector<RenderingCpnt>&       a_outRenderingCpnts);