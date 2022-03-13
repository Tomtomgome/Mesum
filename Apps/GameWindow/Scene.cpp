#include "Scene.hpp"

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

void ComponentManager::initialize()
{
    renderingCpnts.reserve(100);
    transforms.reserve(100);
}

Entity ComponentManager::create_entity()
{
    renderingCpnts.emplace_back();
    transforms.emplace_back();
    return entityCount++;
}

void DrawingData::clean_drawables()
{
    for (auto& drawbles : materialDrawables) { drawbles.clear(); }
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

            DrawableData& drawableData =
                a_outputDrawingData.materialDrawables[rc.materialID]
                    .emplace_back();
            drawableData.color  = {1.0f, 1.0f, 1.0f, 1.0f};
            drawableData.offset = {tc.position.x, tc.position.y, 0.0f};
            drawableData.size   = rc.pictureSize * tc.scale;
            drawableData.angle  = tc.angle;
        }
    }
}
