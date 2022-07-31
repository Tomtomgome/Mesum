#include "Animation.hpp"

#include "Scene.hpp"
#include "GameActionDef.hpp"

#include <imgui.h>

#include <filesystem>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Key::display_gui()
{
    ImGui::DragFloat("Advancement", &advancement, 0.01f, 0.0f, 1.0f);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Modifier::display_gui()
{
    ImGui::ColorPicker4("Color", color.data);
    ImGui::DragFloat2("Offset", offset.data);
    ImGui::DragFloat("Angle", &angle, 0.01f);
    ImGui::DragFloat("Scale", &scale, 0.1f);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Animation::display_gui()
{
    // Modifiers
    mAssert(modifiers.size() == keys.size());
    for (m::mUInt i = 0; i < modifiers.size(); ++i)
    {
        if (ImGui::TreeNode(&modifiers[i], "KeyNode"))
        {
            keys[i].display_gui();
            modifiers[i].display_gui();
            for (auto& ga : gameActionsLists[i]) { ga.display_gui(); }

            if (ImGui::Button("Push Game Action"))
            {
                gameActionsLists[i].emplace_back();
            }
            if (!gameActionsLists[i].empty())
            {
                if (ImGui::Button("Pop Game Action"))
                {
                    gameActionsLists[i].pop_back();
                }
            }

            ImGui::TreePop();
        }
    }

    if (ImGui::Button("AddKey"))
    {
        Key lastKey = keys.back();
        keys.push_back(lastKey);
        Modifier lastModifier = modifiers.back();
        modifiers.push_back(lastModifier);
    }

    auto       numNano  = std::chrono::nanoseconds(animationDuration).count();
    m::mDouble duration = m::mDouble(numNano) / 1000000000.0;
    ImGui::InputDouble("Duration (s)", &duration);
    numNano           = duration * 1000000000;
    animationDuration = std::chrono::nanoseconds(numNano);
    // Update Animation Frame

    m::mInt option = colorMultiply ? 0 : 1;
    ImGui::Combo("Color", &option, "Multiply\0Add\0");
    colorMultiply = option == 0;
    option        = scaleMultiply ? 0 : 1;
    ImGui::Combo("Scale", &option, "Multiply\0Add\0");
    scaleMultiply = option == 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
AnimationBank g_animationBank;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AnimationBank::load()
{
    animations.clear();

    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path animationsPath{currentPath / "data" / "animations"};
    for (const auto& entry :
         std::filesystem::directory_iterator{animationsPath})
    {
        if (entry.path().has_extension() && entry.path().extension() == ".anim")
        {
            std::ifstream inputStream(entry.path());
            Animation     anim;
            anim.name = entry.path().filename().stem().string();
            serialize(anim, inputStream, m_read_flag | m_textual_flag);
            if (anim.ID >= animations.size())
            {
                animations.resize(anim.ID + 1);
            }
            animations[anim.ID] = anim;
        }
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AnimationBank::save()
{
    std::filesystem::path mainPath{std::filesystem::current_path() / "data" /
                                   "animations"};
    for (auto& rAnim : animations)
    {
        std::filesystem::path animationPath{mainPath /
                                            std::string(rAnim.name + ".anim")};

        std::ofstream outputStream(animationPath, std::ios::binary);
        serialize(rAnim, outputStream, m_write_flag | m_textual_flag);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AnimationBank::display_gui()
{
    if (ImGui::Button("Reload"))
    {
        load();
    }
    if (ImGui::Button("Save"))
    {
        save();
    }

    for (auto& rAnime : animations)
    {
        if (ImGui::TreeNode(rAnime.name.c_str()))
        {
            rAnime.display_gui();
            ImGui::TreePop();
        }
    }

    static char name[512] = "";
    ImGui::InputText("New animation name : ", name, 512);

    if (ImGui::Button("Create Animation"))
    {
        animations.emplace_back();
        animations.back().name = name;
        animations.back().ID   = animations.size() - 1;
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AnimationBank::display_animationSelecter(m::mInt& a_animationID)
{
    const char* preview =
        a_animationID < 0 ? "None" : animations[a_animationID].name.c_str();

    if (ImGui::BeginCombo("Played animation", preview))
    {
        for (m::mInt i = -1; i < m::mInt(animations.size()); ++i)
        {
            if (ImGui::Selectable(i == -1 ? "None" : animations[i].name.c_str(),
                                  i == a_animationID))
            {
                a_animationID = i;
            }
        }
        ImGui::EndCombo();
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AnimatorCpnt::display_gui()
{
    ImGui::Checkbox("Animator", &enabled);
    if (!enabled)
    {
        return;
    }

    ImGui::Indent();
    if (ImGui::TreeNode(this, "parameters"))
    {
        m::mInt previousAnimID = animationID;
        g_animationBank.display_animationSelecter(animationID);
        if (previousAnimID != animationID)
        {
            lastKeyIndex       = -1;
            currentAdvancement = 0.0f;
        }
        ImGui::DragFloat("Advancement", &currentAdvancement);
        ImGui::Checkbox("Is Looping", &isLooping);

        ImGui::TreePop();
    }
    ImGui::Unindent();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
auto update(Animation& a_animation, m::mInt const a_lastKeyIndex,
            m::mFloat const& a_currentAdvancement)
{
    m::mU32 nextKeyIndex = a_lastKeyIndex + 1;
    mAssert(nextKeyIndex != a_animation.keys.size());
    Modifier finalModifier;
    Key      previousKey = a_animation.keys[a_lastKeyIndex];
    Key      nextKey     = a_animation.keys[nextKeyIndex];

    m::mFloat relativeAdvancement =
        (a_currentAdvancement - previousKey.advancement) *
        (1.0f / (nextKey.advancement - previousKey.advancement));

    Modifier& previousModifier = a_animation.modifiers[a_lastKeyIndex];
    Modifier& nextModifier     = a_animation.modifiers[nextKeyIndex];

    finalModifier.color =
        (1.0f - relativeAdvancement) * previousModifier.color +
        relativeAdvancement * nextModifier.color;

    finalModifier.offset =
        (1.0f - relativeAdvancement) * previousModifier.offset +
        relativeAdvancement * nextModifier.offset;

    finalModifier.angle =
        (1.0f - relativeAdvancement) * previousModifier.angle +
        relativeAdvancement * nextModifier.angle;

    finalModifier.scale =
        (1.0f - relativeAdvancement) * previousModifier.scale +
        relativeAdvancement * nextModifier.scale;

    return finalModifier;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void process_animatedObjects(
    std::vector<AnimatorCpnt>&                 a_animators,
    std::vector<AnimatorEvent>&                a_animatorsEvents,
    std::chrono::steady_clock::duration const& a_deltaTime)
{
    for (m::mU32 i = 0; i < a_animators.size(); ++i)
    {
        AnimatorCpnt& ra = a_animators[i];
        if (ra.enabled && ra.animationID >= 0)
        {
            Animation& rAnimation = g_animationBank.animations[ra.animationID];

            m::mFloat advancement =
                std::chrono::duration<m::mFloat>(a_deltaTime) /
                std::chrono::duration<m::mFloat>(rAnimation.animationDuration);

            m::mU32 nextKeyIndex;
            Key     nextKey;
            while (advancement > 0.0f)
            {
                // We arrived at the end of the animation
                if (ra.lastKeyIndex + 1 == rAnimation.keys.size())
                {
                    ra.currentAdvancement = 0.0f;
                    ra.lastKeyIndex       = -1;

                    if (!ra.isLooping)
                    {
                        advancement    = 0.0f;
                        ra.animationID = -1;
                    }
                }

                nextKeyIndex = (ra.lastKeyIndex + 1) % rAnimation.keys.size();
                nextKey      = rAnimation.keys[nextKeyIndex];

                if (advancement > nextKey.advancement - ra.currentAdvancement)
                {
                    advancement -=
                        (nextKey.advancement - ra.currentAdvancement);
                    ra.currentAdvancement = nextKey.advancement;
                    ra.lastKeyIndex       = nextKeyIndex;
                    // KeyPassed event
                    AnimatorEvent event;
                    event.type   = AnimatorEvent::Type::keyPassed;
                    event.data.x = i;
                    event.data.y = ra.animationID;
                    event.data.z = ra.lastKeyIndex;
                    a_animatorsEvents.push_back(event);
                }
                else
                {
                    ra.currentAdvancement += advancement;
                    advancement = 0.0f;
                }
            }

            // Animation stopped
            if (ra.animationID == -1)
            {
                continue;
            }

            ra.lastModifier =
                update(rAnimation, ra.lastKeyIndex, ra.currentAdvancement);
        }
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void process_animatorEvents(std::vector<AnimatorEvent>& a_animatorEvents,
                            std::vector<AnimatorCpnt>&  a_animators,
                            std::vector<GameAction*>&   a_gameActions)
{
    for (m::mU32 i = 0; i < a_animatorEvents.size(); ++i)
    {
        AnimatorEvent& rae = a_animatorEvents[i];

        if (rae.type == AnimatorEvent::Type::keyPassed)
        {
            // data.x = animator ID
            // data.y = animation ID
            // data.z = key
            if (rae.data.y >= 0)
            {
                Animation& rAnimation = g_animationBank.animations[rae.data.y];
                // Queue game action
                for (auto gaDesc : rAnimation.gameActionsLists[rae.data.z])
                {
                    switch (gaDesc.type)
                    {
                        case GameAction::Type::selfDestruct:
                        {
                            GASelfDestruct::InternalData actionData{};
                            actionData.entity = rae.data.x;
                            a_gameActions.push_back(
                                GASelfDestruct::create(actionData));
                        }
                        break;
                        default: mNotImplemented; break;
                    }
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void apply_animationModifiers(
    std::vector<AnimatorCpnt> const&  a_animators,
    std::vector<TransformCpnt> const& a_transforms,
    std::vector<RenderingCpnt> const& a_renderingCpnts,
    std::vector<TransformCpnt>&       a_outTransforms,
    std::vector<RenderingCpnt>&       a_outRenderingCpnts)
{
    mAssert(a_animators.size() == a_transforms.size());
    mAssert(a_animators.size() == a_renderingCpnts.size());
    mAssert(a_animators.size() == a_outTransforms.size());
    mAssert(a_animators.size() == a_outRenderingCpnts.size());

    for (m::mU32 i = 0; i < a_animators.size(); ++i)
    {
        RenderingCpnt const& rc = a_renderingCpnts[i];
        AnimatorCpnt const&  ac = a_animators[i];
        TransformCpnt const& tc = a_transforms[i];

        TransformCpnt& etc = a_outTransforms[i];
        etc                = tc;
        RenderingCpnt& erc = a_outRenderingCpnts[i];
        erc                = rc;

        if (ac.enabled && ac.animationID >= 0)
        {
            Modifier modifier = ac.lastModifier;  // pAnimation->lastModifier;
            m::mBool colorMultiply = g_animationBank.animations[ac.animationID]
                                         .colorMultiply;  // Otherwise Add
            m::mBool scaleMultiply = g_animationBank.animations[ac.animationID]
                                         .scaleMultiply;  // Otherwise Add

            if (tc.enabled)
            {
                apply_modifierToTC(etc, modifier, scaleMultiply);
            }
            if (rc.enabled)
            {
                apply_modifierToRC(erc, modifier, colorMultiply);
            }
        }
    }
}