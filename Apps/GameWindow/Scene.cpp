#include <imgui.h>
#include "Scene.hpp"

#include <filesystem>

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
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
            std::ifstream                      inputStream(entry.path());
            m::serializer::mSerializerIfstream serializer(inputStream);
            Model                              model;
            model.name = entry.path().filename().stem().string();
            mSerialize(model, serializer);
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
    for (auto& rModel : models)
    {
        std::filesystem::path modelPath{mainPath /
                                        std::string(rModel.name + ".model")};

        std::ofstream outputStream(modelPath, std::ios::binary);
        m::serializer::mSerializerOfstream serializer(outputStream);
        mSerialize(rModel, serializer);
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
void ModelBank::display_selecter(ModelBank::ModelID& a_modelID)
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

template <>
void ComponentManager::enable_component<DisplacerCpnt>(
    Entity const& a_entity, DisplacerCpnt const& a_component)
{
    displacers[a_entity]         = a_component;
    displacers[a_entity].enabled = true;
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

    ImGui::Text("allocated entities : %d", entityCount);
    ImGui::Text("active entities : %d",
                entityCount - m::mInt(freeEntities.size()));

    for (m::mUInt i = 0; i < entityCount; ++i)
    {
        if (enabled[i] == 1)
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
}

void ComponentManager::initialize()
{
    renderingCpnts.reserve(100);
    transforms.reserve(100);
    animators.reserve(100);
    collisions.reserve(100);
    enabled.reserve(100);
}

void ComponentManager::reset()
{
    entityCount = 0;
    renderingCpnts.clear();
    transforms.clear();
    animators.clear();
    collisions.clear();
    enabled.clear();
    freeEntities.clear();
}

void ComponentManager::load_fromFile(std::string const& a_path)
{
    std::ifstream inputStream(a_path);
    m::mU32       version;
    std::string   debugName;
    inputStream >> debugName >> version;

    m::serializer::mSerializerIfstream serializer(inputStream);

    if (version >= 1)
    {
        inputStream >> entityCount;

        renderingCpnts.resize(entityCount);
        animators.resize(entityCount);
        transforms.resize(entityCount);
        collisions.resize(entityCount);
        enabled.resize(entityCount);

        for (int i = 0; i < entityCount; ++i)
        {
            mSerialize(renderingCpnts[i], serializer);
            mSerialize(animators[i], serializer);
            mSerialize(transforms[i], serializer);
            if (version >= 2)
            {
                mSerialize(collisions[i], serializer);
            }
            if (version >= 3)
            {
                inputStream >> enabled[i];
                if (enabled[i] == 0)
                {
                    freeEntities.push_back(i);
                }
            }
            else
            {
                enabled[i] = 1;
            }
        }
    }
}

void ComponentManager::save_toFile(std::string const& a_path)
{
    std::ofstream outputStream(a_path, std::ios::binary);
    outputStream << "CpntManager: " << s_version << ' ';

    m::serializer::mSerializerOfstream serializer(outputStream);
    outputStream << entityCount << std::endl;
    for (int i = 0; i < entityCount; ++i)
    {
        mSerialize(renderingCpnts[i], serializer);
        mSerialize(animators[i], serializer);
        mSerialize(transforms[i], serializer);
        mSerialize(collisions[i], serializer);
        outputStream << enabled[i] << std::endl;
    }
}

Entity ComponentManager::create_entity()
{
    if (freeEntities.size() > 0)
    {
        Entity e = freeEntities.back();
        freeEntities.pop_back();
        enabled[e] = 1;
        return e;
    }

    renderingCpnts.emplace_back();
    transforms.emplace_back();
    animators.emplace_back();
    collisions.emplace_back();
    enabled.emplace_back(1);
    return entityCount++;
}

Entity ComponentManager::create_entityFromModel(
    Model const& a_model, TransformCpnt const& a_creationTransform)
{
    Entity e;

    if (freeEntities.empty())
    {
        renderingCpnts.emplace_back(a_model.renderingCpnt);
        transforms.emplace_back(
            apply_transformToTC(a_model.transform, a_creationTransform));
        animators.emplace_back(a_model.animator);
        collisions.emplace_back(a_model.collision);
        enabled.emplace_back(1);

        e = entityCount++;
    }
    else
    {
        e = freeEntities.back();
        freeEntities.pop_back();

        renderingCpnts[e] = a_model.renderingCpnt;
        transforms[e] =
            apply_transformToTC(a_model.transform, a_creationTransform);
        animators[e]  = a_model.animator;
        collisions[e] = a_model.collision;
        enabled[e]    = 1;
    }

    return e;
}

void ComponentManager::kill_entity(Entity const a_entity)
{
    freeEntities.push_back(a_entity);

    renderingCpnts[a_entity].enabled = false;
    animators[a_entity].enabled      = false;
    transforms[a_entity].enabled     = false;
    collisions[a_entity].enabled     = false;
    enabled[a_entity]                = 0;
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
