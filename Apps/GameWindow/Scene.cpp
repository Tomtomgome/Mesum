#include <imgui.h>
#include "Scene.hpp"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void RenderingCpnt::read(std::ifstream& a_inputStream)
{
    m::mU32     version;
    std::string debugName;
    a_inputStream >> debugName >> version;

    if (version <= 1)
    {
        a_inputStream >> color.x >> color.y >> color.z >> color.w;
        a_inputStream >> materialID;
        a_inputStream >> pictureSize;
        a_inputStream >> enabled;
    }
}

void RenderingCpnt::write(std::ofstream& a_outputStream) const
{
    a_outputStream << "RenderingCpnt: " << s_version << ' ';

    a_outputStream << color.x << ' ' << color.y << ' ' << color.z << ' '
                   << color.w << ' ';
    a_outputStream << materialID << ' ';
    a_outputStream << pictureSize << ' ';
    a_outputStream << enabled << std::endl;
}

void RenderingCpnt::display_gui()
{
    ImGui::Checkbox("Rendering", &enabled);
    if (!enabled)
    {
        return;
    }

    ImGui::Indent();
    if (ImGui::TreeNode(this, "parameters"))
    {
        ImGui::ColorPicker4("Color", color.data);
        // Be carefull with this ?
        ImGui::InputInt("MaterialID", (m::mInt*)(&materialID));
        ImGui::InputInt("Texture Size", (m::mInt*)(&pictureSize));

        ImGui::TreePop();
    }
    ImGui::Unindent();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TransformCpnt::read(std::ifstream& a_inputStream)
{
    m::mU32     version;
    std::string debugName;
    a_inputStream >> debugName >> version;

    if (version <= 1)
    {
        a_inputStream >> position.x >> position.y;
        a_inputStream >> angle;
        a_inputStream >> scale;
        a_inputStream >> enabled;
    }
}

void TransformCpnt::write(std::ofstream& a_outputStream) const
{
    a_outputStream << "TransformCpnt: " << s_version << ' ';

    a_outputStream << position.x << ' ' << position.y << ' ';
    a_outputStream << angle << ' ';
    a_outputStream << scale << ' ';
    a_outputStream << enabled << std::endl;
}

void TransformCpnt::display_gui()
{
    ImGui::Checkbox("Transform", &enabled);
    if (!enabled)
    {
        return;
    }

    ImGui::Indent();
    if (ImGui::TreeNode(this, "parameters"))
    {
        ImGui::DragFloat2("Position", position.data);
        ImGui::DragFloat("angle", &angle, 0.01f);
        ImGui::DragFloat("scale", &scale);
        ImGui::TreePop();
    }
    ImGui::Unindent();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void Key::read(std::ifstream& a_inputStream)
{
    m::mU32     version;
    std::string debugName;
    a_inputStream >> debugName >> version;

    if (version <= 1)
    {
        a_inputStream >> advancement;
    }
}

void Key::write(std::ofstream& a_outputStream) const
{
    a_outputStream << "AnimationKey: " << s_version << " ";

    a_outputStream << advancement << std::endl;
}

void Modifier::read(std::ifstream& a_inputStream)
{
    m::mU32     version;
    std::string debugName;
    a_inputStream >> debugName >> version;

    if (version <= 1)
    {
        a_inputStream >> color.x >> color.y >> color.z >> color.w;
        a_inputStream >> offset.x >> offset.y;
        a_inputStream >> angle;
        a_inputStream >> scale;
    }
}

void Modifier::write(std::ofstream& a_outputStream) const
{
    a_outputStream << "AnimationModifer: " << s_version << ' ';

    a_outputStream << color.x << ' ' << color.y << ' ' << color.z << ' '
                   << color.w << ' ';
    a_outputStream << offset.x << ' ' << offset.y << ' ';
    a_outputStream << angle << ' ';
    a_outputStream << scale << std::endl;
}

void Animation::read(std::ifstream& a_inputStream)
{
    m::mU32     version;
    std::string debugName;
    a_inputStream >> debugName >> version;

    if (version <= 1)
    {
        m::mU32 size;
        a_inputStream >> size;
        keys.resize(size);
        for (auto& key : keys) { key.read(a_inputStream); }
        a_inputStream >> size;
        modifiers.resize(size);
        for (auto& modif : modifiers) { modif.read(a_inputStream); }
        m::mU64 tmpDuration;
        a_inputStream >> tmpDuration;
        animationDuration = std::chrono::nanoseconds(tmpDuration);
        a_inputStream >> lastKeyIndex;
        lastModifier.read(a_inputStream);
        a_inputStream >> currentAdvancement;
        a_inputStream >> isLooping;
        a_inputStream >> colorMultiply;
        a_inputStream >> scaleMultiply;
    }
}

void Animation::write(std::ofstream& a_outputStream) const
{
    a_outputStream << "Animation: " << s_version << ' ';

    a_outputStream << keys.size() << ' ';
    for (auto& key : keys) { key.write(a_outputStream); }
    a_outputStream << modifiers.size() << ' ';
    for (auto& modif : modifiers) { modif.write(a_outputStream); }
    a_outputStream << m::mU64(duration_cast<std::chrono::nanoseconds>(
                                  animationDuration)
                                  .count())
                   << ' ';
    a_outputStream << lastKeyIndex << ' ';
    lastModifier.write(a_outputStream);
    a_outputStream << currentAdvancement << ' ';
    a_outputStream << isLooping << ' ';
    a_outputStream << colorMultiply << ' ';
    a_outputStream << scaleMultiply << std::endl;
}

void Animation::display_gui()
{
    // Modifiers
    // keys

    auto numNano = std::chrono::nanoseconds(animationDuration).count();
    m::mDouble duration = m::mDouble(numNano)/1000000000.0;
    ImGui::InputDouble("Duration (s)", &duration);
    numNano = duration * 1000000000;
    animationDuration = std::chrono::nanoseconds(numNano);
    //Update Animation Frame

    ImGui::DragFloat("Advancement", &currentAdvancement);
    ImGui::Checkbox("Is Looping", &isLooping);
    m::mInt option = colorMultiply ? 0 : 1;
    ImGui::Combo("Color", &option, "Multiply\0Add\0");
    colorMultiply = option == 0;
    option = scaleMultiply ? 0 : 1;
    ImGui::Combo("Scale", &option, "Multiply\0Add\0");
    scaleMultiply = option == 0;

}

void AnimatorCpnt::read(std::ifstream& a_inputStream)
{
    m::mU32     version;
    std::string debugName;
    a_inputStream >> debugName >> version;

    if (version <= 1)
    {
        bool hasAnimation;
        a_inputStream >> hasAnimation;
        if (hasAnimation)
        {
            if (pAnimation == nullptr)
            {
                pAnimation = new Animation();
            }
            pAnimation->read(a_inputStream);
        }
        a_inputStream >> enabled;
    }
}

void AnimatorCpnt::write(std::ofstream& a_outputStream) const
{
    a_outputStream << "AnimatorCpnt: " << s_version << ' ';

    if (pAnimation != nullptr)
    {
        a_outputStream << true << ' ';
        pAnimation->write(a_outputStream);
    }
    else
    {
        a_outputStream << false << ' ';
    }
    a_outputStream << enabled << std::endl;
}

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
        if(pAnimation == nullptr)
        {
            if(ImGui::Button("Create Animation")) {
                pAnimation = new Animation();
            }
        }
        else
        {
            pAnimation->display_gui();
        }

        ImGui::TreePop();
    }
    ImGui::Unindent();
}

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

void ComponentManager::display_gui()
{
    if(ImGui::Button("Add Entity"))
    {
        create_entity();
    }

    ImGui::Text("Total entity : %d", entityCount);

    for (m::mUInt i = 0; i < entityCount; ++i)
    {
        std::stringstream entityName;
        entityName << "Entity " << i;
        if (ImGui::TreeNode(entityName.str().c_str()))
        {
            transforms[i].display_gui();
            renderingCpnts[i].display_gui();
            animators[i].display_gui();
            ImGui::TreePop();
        }
    }
}

void ComponentManager::initialize()
{
    renderingCpnts.reserve(100);
    transforms.reserve(100);
    animators.reserve(100);
}

void ComponentManager::reset()
{
    entityCount = 0;
    renderingCpnts.clear();
    transforms.clear();

    for (auto& animator : animators)
    {
        if (animator.pAnimation != nullptr)
        {
            delete animator.pAnimation;
        }
    }
    animators.clear();
}

void ComponentManager::load_fromCopy(ComponentManager const& a_source)
{
    reset();
    entityCount    = a_source.entityCount;
    renderingCpnts = a_source.renderingCpnts;
    transforms     = a_source.transforms;
    animators      = a_source.animators;

    for (auto& animator : animators)
    {
        if (animator.pAnimation != nullptr)
        {
            Animation& tmpAnim     = *animator.pAnimation;
            animator.pAnimation    = new Animation();
            *(animator.pAnimation) = tmpAnim;
        }
    }
}
void ComponentManager::load_fromFile(std::string const& a_path)
{
    std::ifstream inputStream(a_path);
    m::mU32       version;
    std::string   debugName;
    inputStream >> debugName >> version;

    if (version <= 1)
    {
        inputStream >> entityCount;

        renderingCpnts.resize(entityCount);
        animators.resize(entityCount);
        transforms.resize(entityCount);

        for (int i = 0; i < entityCount; ++i)
        {
            renderingCpnts[i].read(inputStream);
            animators[i].read(inputStream);
            transforms[i].read(inputStream);
        }
    }
}

void ComponentManager::save_toFile(std::string const& a_path) const
{
    std::ofstream outputStream(a_path, std::ios::binary);
    outputStream << "CpntManager: " << s_version << ' ';

    outputStream << entityCount << std::endl;
    for (int i = 0; i < entityCount; ++i)
    {
        renderingCpnts[i].write(outputStream);
        animators[i].write(outputStream);
        transforms[i].write(outputStream);
    }
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
            if (ac.enabled && ac.pAnimation != nullptr)
            {
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