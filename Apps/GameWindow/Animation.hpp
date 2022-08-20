#pragma once

#include "GameAction.hpp"
#include <Kernel/Serializable.hpp>
#include <Kernel/Math.hpp>

#include <vector>

struct RenderingCpnt;
struct TransformCpnt;
struct ComponentManager;
using Entity = m::mU32;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct Key
{
    void display_gui();

    m::mFloat advancement = 0;
};
mBegin_serialization(Key, 1);

mSerialize_memberFrom(1, advancement);

mEnd_serialization(Key);

struct Modifier
{
    void display_gui();

    m::math::mVec4 color{};
    m::math::mVec2 offset{};
    m::mFloat      angle = 0;
    m::mFloat      scale = 0;
};
mBegin_serialization(Modifier, 1);

mSerialize_memberFrom(1, color.r);
mSerialize_memberFrom(1, color.g);
mSerialize_memberFrom(1, color.b);
mSerialize_memberFrom(1, color.a);
mSerialize_memberFrom(1, offset.x);
mSerialize_memberFrom(1, offset.y);
mSerialize_memberFrom(1, angle);
mSerialize_memberFrom(1, scale);

mEnd_serialization(Modifier);

struct Animation
{
    void display_gui();

    std::string                              name{"unnamed"};
    m::mUInt                                 ID{0};
    std::vector<Key>                         keys{{0.0f}, {1.0f}};
    std::vector<Modifier>                    modifiers{{}, {}};
    std::vector<std::vector<GameActionDesc>> gameActionsLists{{}, {}};
    std::chrono::steady_clock::duration      animationDuration{};
    m::mBool colorMultiply = true;  // Otherwise Add
    m::mBool scaleMultiply = true;  // Otherwise Add
};
mBegin_serialization(Animation, 1);

mSerialize_memberFrom(1, ID);

m::mUInt nbKeys = a_object.keys.size();
mSerialize_from(1, nbKeys);
a_object.keys.resize(nbKeys);
a_object.modifiers.resize(nbKeys);
a_object.gameActionsLists.resize(nbKeys);

for (auto& key : a_object.keys) { mSerialize_from(1, key); }

for (auto& modifier : a_object.modifiers) { mSerialize_from(1, modifier); }

for (auto& gameActionList : a_object.gameActionsLists)
{
    m::mUInt nbGameAction = gameActionList.size();
    mSerialize_from(1, nbGameAction);
    gameActionList.resize(nbGameAction);
    for (auto& gameAction : gameActionList) { mSerialize_from(1, gameAction); }
}
m::mU64 duration =
    (m::mU64(duration_cast<std::chrono::nanoseconds>(a_object.animationDuration)
                 .count()));
mSerialize_from(1, duration);
a_object.animationDuration = std::chrono::nanoseconds(duration);

mSerialize_memberFrom(1, colorMultiply);
mSerialize_memberFrom(1, scaleMultiply);

mEnd_serialization(Animation);

struct AnimationBank
{
    void load();
    void unload();
    void save();

    void display_gui();
    void display_animationSelecter(m::mInt& a_animationID);

    std::vector<Animation> animations;
};

extern AnimationBank g_animationBank;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct AnimatorEvent
{
    enum Type
    {
        none      = 0,
        keyPassed = 1,
    };

    Type            type{none};
    m::math::mIVec3 data{0, 0, 0};
};

struct AnimatorCpnt
{
    void display_gui();

    m::mInt   animationID{-1};
    Modifier  lastModifier{};
    m::mInt   lastKeyIndex{0};
    m::mFloat currentAdvancement{0};
    m::mBool  isLooping{true};
    m::mBool  enabled{false};
};
mBegin_serialization(AnimatorCpnt, 1);

mSerialize_memberFrom(1, animationID);
mSerialize_memberFrom(1, lastModifier);
mSerialize_memberFrom(1, lastKeyIndex);
mSerialize_memberFrom(1, currentAdvancement);
mSerialize_memberFrom(1, isLooping);
mSerialize_memberFrom(1, enabled);

mEnd_serialization(AnimatorCpnt);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void apply_modifierToTC(TransformCpnt& a_tc, Modifier const& a_modifier,
                        bool a_scaleMultiply);
void apply_modifierToRC(RenderingCpnt& a_rc, Modifier const& a_modifier,
                        bool a_colorMultiply);

void process_animatedObjects(
    std::vector<AnimatorCpnt>&                 a_animators,
    std::vector<AnimatorEvent>&                a_animatorEvents,
    std::chrono::steady_clock::duration const& a_deltaTime);

void process_animatorEvents(std::vector<AnimatorEvent>& a_animatorEvents,
                            std::vector<AnimatorCpnt>&  a_animators,
                            std::vector<GameAction*>&   a_gameActions);

void apply_animationModifiers(
    std::vector<AnimatorCpnt> const&  a_animators,
    std::vector<TransformCpnt> const& a_transforms,
    std::vector<RenderingCpnt> const& a_renderingCpnts,
    std::vector<TransformCpnt>&       a_outTransforms,
    std::vector<RenderingCpnt>&       a_outRenderingCpnts);