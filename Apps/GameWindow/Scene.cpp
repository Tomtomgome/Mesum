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
        m::mFloat degreeAngle = 360 * (angle / (2 * 3.141592));
        ImGui::DragFloat("angle", &degreeAngle, 0.01f);
        angle = (degreeAngle / 360) * 2 * 3.141592;
        ImGui::DragFloat("scale", &scale, 0.1f);
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

template <>
void ComponentManager::enable_component<CollisionCpnt>(
    Entity const& a_entity, CollisionCpnt const& a_component)
{
    collisions[a_entity]         = a_component;
    collisions[a_entity].enabled = true;
}

void ComponentManager::display_gui()
{
    if (ImGui::Button("Add Entity"))
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
            ImGui::PushItemWidth(-80);
            transforms[i].display_gui();
            renderingCpnts[i].display_gui();
            animators[i].display_gui();
            collisions[i].display_gui();
            ImGui::PopItemWidth();
            ImGui::TreePop();
        }
    }
}

void ComponentManager::initialize()
{
    renderingCpnts.reserve(100);
    transforms.reserve(100);
    animators.reserve(100);
    collisions.reserve(100);
}

void ComponentManager::reset()
{
    entityCount = 0;
    renderingCpnts.clear();
    transforms.clear();
    animators.clear();
    collisions.clear();
}

void ComponentManager::load_fromCopy(ComponentManager const& a_source)
{
    reset();
    entityCount    = a_source.entityCount;
    renderingCpnts = a_source.renderingCpnts;
    transforms     = a_source.transforms;
    animators      = a_source.animators;
    collisions     = a_source.collisions;
}

void ComponentManager::load_fromFile(std::string const& a_path)
{
    std::ifstream inputStream(a_path);
    m::mU32       version;
    std::string   debugName;
    inputStream >> debugName >> version;

    if (version >= 1)
    {
        inputStream >> entityCount;

        renderingCpnts.resize(entityCount);
        animators.resize(entityCount);
        transforms.resize(entityCount);
        collisions.resize(entityCount);

        for (int i = 0; i < entityCount; ++i)
        {
            renderingCpnts[i].read(inputStream);
            animators[i].read(inputStream);
            transforms[i].read(inputStream);
            if (s_version >= 2)
            {
                collisions[i].read(inputStream);
            }
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
        collisions[i].write(outputStream);
    }
}

Entity ComponentManager::create_entity()
{
    renderingCpnts.emplace_back();
    transforms.emplace_back();
    animators.emplace_back();
    collisions.emplace_back();
    return entityCount++;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void DrawingData::clean_drawables()
{
    for (auto& drawbles : materialDrawables) { drawbles.clear(); }
}

void process_renderableObjects(
    std::vector<TransformCpnt> const& a_transforms,
    std::vector<RenderingCpnt> const& a_renderingCpnts,
    DrawingData&                      a_outputDrawingData)
{
    mAssert(a_transforms.size() == a_renderingCpnts.size());

    for (m::mU32 i = 0; i < a_transforms.size(); ++i)
    {
        RenderingCpnt const& rc = a_renderingCpnts[i];
        TransformCpnt const& tc = a_transforms[i];
        if (rc.enabled && tc.enabled)
        {
            if (rc.materialID + 1 >
                a_outputDrawingData.materialDrawables.size())
            {
                a_outputDrawingData.materialDrawables.resize(rc.materialID + 1);
            }

            DrawableData& drawableData =
                a_outputDrawingData.materialDrawables[rc.materialID]
                    .emplace_back();
            drawableData.color  = rc.color;
            drawableData.offset = {tc.position.x, tc.position.y, 0.0f};
            drawableData.size   = rc.pictureSize * tc.scale;
            drawableData.angle  = tc.angle;
        }
    }
}

