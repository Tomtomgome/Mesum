#include <imgui.h>
#include "Scene.hpp"

#include <filesystem>
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

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
TransformCpnt apply_transformToTC(TransformCpnt const& a_transformA,
                                  TransformCpnt const& a_transformB)
{
    TransformCpnt outTransform = a_transformA;
    outTransform.position += a_transformB.position;
    outTransform.angle += a_transformB.angle;
    outTransform.scale *= a_transformB.scale;

    return outTransform;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Model::read(std::ifstream& a_inputStream)
{
    m::mU32     version;
    std::string debugName;
    a_inputStream >> debugName >> version;

    if (version <= 1)
    {
        a_inputStream >> name;
        renderingCpnt.read(a_inputStream);
        animator.read(a_inputStream);
        transform.read(a_inputStream);
        collision.read(a_inputStream);
        a_inputStream >> ID;
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Model::write(std::ofstream& a_outputStream) const
{
    a_outputStream << "Model: " << s_version << ' ';

    a_outputStream << name << std::endl;
    renderingCpnt.write(a_outputStream);
    animator.write(a_outputStream);
    transform.write(a_outputStream);
    collision.write(a_outputStream);
    a_outputStream << ID << std::endl;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Model::display_gui()
{
    renderingCpnt.display_gui();
    animator.display_gui();
    transform.display_gui();
    collision.display_gui();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ModelBank g_modelBank;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ModelBank::load()
{
    models.clear();
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path modelsPath{currentPath / "data" / "models"};
    for (const auto& entry : std::filesystem::directory_iterator{modelsPath})
    {
        if (entry.path().has_extension() &&
            entry.path().extension() == ".model")
        {
            std::ifstream inputStream(entry.path());
            Model         model;
            model.name = entry.path().filename().stem().string();
            model.read(inputStream);
            if (model.ID >= models.size())
            {
                models.resize(model.ID + 1);
            }
            models[model.ID] = model;
        }
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ModelBank::save()
{
    std::filesystem::path mainPath{std::filesystem::current_path() / "data" /
                                   "models"};
    for (auto const& rModel : models)
    {
        std::filesystem::path modelPath{mainPath /
                                        std::string(rModel.name + ".model")};

        std::ofstream outputStream(modelPath, std::ios::binary);
        rModel.write(outputStream);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ModelBank::display_gui()
{
    if (ImGui::Button("Reload"))
    {
        load();
    }
    if (ImGui::Button("Save"))
    {
        save();
    }

    for (auto& rModel : models)
    {
        if (ImGui::TreeNode(rModel.name.c_str()))
        {
            rModel.display_gui();
            ImGui::TreePop();
        }
    }

    static char name[512] = "";
    ImGui::InputText("New model name : ", name, 512);

    if (ImGui::Button("Create Model"))
    {
        models.emplace_back();
        models.back().name = name;
        models.back().ID   = models.size() - 1;
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ModelBank::display_selecter(m::mInt& a_modelID)
{
    const char* preview = (models.size() == 0 || a_modelID < 0)
                              ? "None"
                              : models[a_modelID].name.c_str();

    if (ImGui::BeginCombo("Played animation", preview))
    {
        for (m::mInt i = -1; i < m::mInt(models.size()); ++i)
        {
            if (ImGui::Selectable(i == -1 ? "None" : models[i].name.c_str(),
                                  i == a_modelID))
            {
                a_modelID = i;
            }
        }
        ImGui::EndCombo();
    }
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
    static TransformCpnt creationTransform;
    static m::mInt       modelID;
    creationTransform.display_gui();
    g_modelBank.display_selecter(modelID);
    if (ImGui::Button("Add Entity From Model"))
    {
        create_entityFromModel(g_modelBank.models[modelID], creationTransform);
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

Entity ComponentManager::create_entityFromModel(
    Model const& a_model, TransformCpnt const& a_creationTransform)
{
    renderingCpnts.emplace_back(a_model.renderingCpnt);

    transforms.emplace_back(
        apply_transformToTC(a_model.transform, a_creationTransform));
    ;

    animators.emplace_back(a_model.animator);
    collisions.emplace_back(a_model.collision);
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
