#include "Scene.hpp"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
template <>
void ComponentManager::enable_component<RenderingCpnt>(
    Entity const& a_entity, RenderingCpnt const& a_component)
{
    renderingCpnts[a_entity]         = a_component;
    renderingCpnts[a_entity].enabled = true;
}

template <>
void ComponentManager::enable_component<TransformCpnt>(
    Entity const& a_entity, TransformCpnt const& a_component)
{
    transforms[a_entity]         = a_component;
    transforms[a_entity].enabled = true;
}

template <>
void ComponentManager::enable_component<AnimatorCpnt>(
    Entity const& a_entity, AnimatorCpnt const& a_component)
{
    animators[a_entity]         = a_component;
    animators[a_entity].enabled = true;
}

void ComponentManager::initialize()
{
    renderingCpnts.reserve(100);
    transforms.reserve(100);
    animators.reserve(100);
}

Entity ComponentManager::create_entity()
{
    renderingCpnts.emplace_back();
    transforms.emplace_back();
    animators.emplace_back();
    return entityCount++;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void DrawingData::clean_drawables()
{
    for (auto& drawbles : materialDrawables) { drawbles.clear(); }
}

void apply_modifierToRC(RenderingCpnt& a_rc, Modifier const& a_modifier,
                        bool a_colorMultiply)
{
    if (a_colorMultiply)
    {
        a_rc.color *= a_modifier.color;
    }
    else
    {
        a_rc.color += a_modifier.color;
    }
}

void apply_modifierToTC(TransformCpnt& a_tc, Modifier const& a_modifier,
                        bool a_scaleMultiply)
{
    if (a_scaleMultiply)
    {
        a_tc.scale *= a_modifier.scale;
    }
    else
    {
        a_tc.scale += a_modifier.scale;
    }

    a_tc.position += a_modifier.offset;
    a_tc.angle += a_modifier.angle;
}

void process_renderableObjects(ComponentManager const& a_cpntManager,
                               DrawingData&            a_outputDrawingData)
{
    for (m::mU32 i = 0; i < a_cpntManager.entityCount; ++i)
    {
        RenderingCpnt const& rc = a_cpntManager.renderingCpnts[i];
        TransformCpnt const& tc = a_cpntManager.transforms[i];
        if (rc.enabled && tc.enabled)
        {
            if (rc.materialID + 1 >
                a_outputDrawingData.materialDrawables.size())
            {
                a_outputDrawingData.materialDrawables.resize(rc.materialID + 1);
            }

            Modifier modifier;
            m::mBool colorMultiply = false;  // Otherwise Add
            m::mBool scaleMultiply = false;

            AnimatorCpnt const& ac = a_cpntManager.animators[i];
            if (ac.enabled)
            {
                mAssert(ac.pAnimation != nullptr);
                modifier      = ac.pAnimation->lastModifier;
                colorMultiply = ac.pAnimation->colorMultiply;
                scaleMultiply = ac.pAnimation->scaleMultiply;
            }

            RenderingCpnt effectiveRc = rc;
            apply_modifierToRC(effectiveRc, modifier, colorMultiply);
            TransformCpnt effectiveTc = tc;
            apply_modifierToTC(effectiveTc, modifier, scaleMultiply);

            DrawableData& drawableData =
                a_outputDrawingData.materialDrawables[effectiveRc.materialID]
                    .emplace_back();
            drawableData.color  = effectiveRc.color;
            drawableData.offset = {effectiveTc.position.x,
                                   effectiveTc.position.y, 0.0f};
            drawableData.size   = effectiveRc.pictureSize * effectiveTc.scale;
            drawableData.angle  = effectiveTc.angle;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool update(Animation&                                 a_animation,
            std::chrono::steady_clock::duration const& a_deltaTime)
{
    m::mFloat advancement =
        std::chrono::duration<m::mFloat>(a_deltaTime) /
        std::chrono::duration<m::mFloat>(a_animation.animationDuration);
    a_animation.currentAdvancement += advancement;

    if (a_animation.currentAdvancement > 1)
    {
        if (!a_animation.isLooping)
        {
            return false;
        }

        a_animation.lastKeyIndex = 0;
        a_animation.currentAdvancement =
            std::modf(1.0f, &a_animation.currentAdvancement);
    }

    m::mU32 nextKeyIndex = a_animation.lastKeyIndex + 1;
    Key     nextKey;
    while (nextKeyIndex < a_animation.keys.size())
    {
        nextKey = a_animation.keys[nextKeyIndex];
        if (nextKey.advancement >= a_animation.currentAdvancement)
        {
            break;
        }
        ++nextKeyIndex;
    }
    if (nextKeyIndex == a_animation.keys.size())
    {
        mInterrupt;
    }
    a_animation.lastKeyIndex = nextKeyIndex - 1;
    Key previousKey          = a_animation.keys[a_animation.lastKeyIndex];

    m::mFloat relativeAdvancement =
        (a_animation.currentAdvancement - previousKey.advancement) *
        (1.0 / (nextKey.advancement - previousKey.advancement));

    Modifier& previousModifier =
        a_animation.modifiers[a_animation.lastKeyIndex];
    Modifier& nextModifier = a_animation.modifiers[nextKeyIndex];

    a_animation.lastModifier.color =
        (1.0f - relativeAdvancement) * previousModifier.color +
        relativeAdvancement * nextModifier.color;

    a_animation.lastModifier.offset =
        (1.0f - relativeAdvancement) * previousModifier.offset +
        relativeAdvancement * nextModifier.offset;

    a_animation.lastModifier.angle =
        (1.0f - relativeAdvancement) * previousModifier.angle +
        relativeAdvancement * nextModifier.angle;

    a_animation.lastModifier.scale =
        (1.0f - relativeAdvancement) * previousModifier.scale +
        relativeAdvancement * nextModifier.scale;

    return true;
}

void process_animatedObjects(
    ComponentManager&                          a_cpntManager,
    std::chrono::steady_clock::duration const& a_deltaTime)
{
    for (m::mU32 i = 0; i < a_cpntManager.entityCount; ++i)
    {
        AnimatorCpnt& ra = a_cpntManager.animators[i];
        if (ra.enabled)
        {
            mAssert(ra.pAnimation != nullptr);
            Animation& rAnimation = *ra.pAnimation;
            ra.enabled            = update(rAnimation, a_deltaTime);
        }
    }
}