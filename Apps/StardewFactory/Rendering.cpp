#include <MesumGraphics/DearImgui/imgui.h>

#include <Rendering.hpp>

using namespace m;

//*****************************************************************************
// Displays
//*****************************************************************************

WorldToDisplay g_world;

void WorldToDisplay::display_layer(std::vector<DrawData>& a_layer)
{
    const ImVec2 p         = ImGui::GetCursorScreenPos();
    mFloat        cx        = p.x + 5.0f;
    mFloat        cy        = p.y + 5.0f;
    ImDrawList*  draw_list = ImGui::GetWindowDrawList();

    for (auto drawData : a_layer)
    {
        mFloat       tX  = cx + drawData.m_position.x;
        mFloat       tY  = cy + drawData.m_position.y;
        const ImU32 col = ImColor(drawData.m_color.x, drawData.m_color.y,
                                  drawData.m_color.z, drawData.m_color.w);
        if (drawData.m_filled)
        {
            draw_list->AddRectFilled(
                ImVec2(tX, tY),
                ImVec2(tX + drawData.m_size.x, tY + drawData.m_size.y), col);
        }
        else
        {
            draw_list->AddRect(
                ImVec2(tX, tY),
                ImVec2(tX + drawData.m_size.x, tY + drawData.m_size.y), col);
        }
    }

    mU64 cap = a_layer.capacity();
    a_layer.clear();
    a_layer.reserve(cap);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// void render_agent(AgentNutriment const* a_nutriment)
// {
//     g_world.m_backLayer.emplace_back();
//     DrawData& data = g_world.m_backLayer.back();
//
//     data.m_position.x =
//         (parcelSize + parcelPadding) * a_nutriment->m_position.x;
//     data.m_position.y =
//         (parcelSize + parcelPadding) * a_nutriment->m_position.y;
//
//     data.m_size  = {parcelSize, parcelSize};
//     data.m_color = {colField.x, colField.y, colField.z,
//                     a_nutriment->m_nutrientQuantity / s_fieldMaxNutiments};
// }

static ImVec4 colField = {0.284, 0.159, 0.0f, 1.0f};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void render_agent(AgentSoil const* a_soil)
{
    g_world.m_backLayer.emplace_back();
    DrawData& data = g_world.m_backLayer.back();

    data.m_position.x = (parcelSize + parcelPadding) * a_soil->m_position.x;
    data.m_position.y = (parcelSize + parcelPadding) * a_soil->m_position.y;

    math::mVec3 color;
    switch (a_soil->m_type)
    {
        case AgentSoil::Dirt:
        {
            color = {colField.x, colField.y, colField.z};
        }
        break;
        case AgentSoil::WetDirt:
        {
            color = {colField.x, colField.y, colField.z};
            color *= 0.9f;
        }
        break;
        case AgentSoil::Grass:
        {
            color = {colField.x, 2.0f * colField.y, colField.z};
        }
        break;
        default: break;
    }

    data.m_size  = {parcelSize, parcelSize};
    data.m_color = {color.x, color.y, color.z,
                    a_soil->m_nutrientQuantity / s_fieldMaxNutiments};
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// void render_agent(AgentPlant const* a_agent)
// {
//     if (a_agent->m_isHarvested)
//     {
//         return;
//     }
//
//     g_world.m_middleLayer.emplace_back();
//     DrawData& data = g_world.m_middleLayer.back();
//     mFloat     agentSize =
//         math::lerp(agentSizeSmall, agentSizeBig, a_agent->m_age /
//         s_matureAge);
//     mFloat innerCellPadding = parcelSize / 2.0f - agentSize / 2.0f;
//     data.m_position.x =
//         (parcelSize + parcelPadding) * a_agent->m_position.x +
//         innerCellPadding;
//     data.m_position.y =
//         (parcelSize + parcelPadding) * a_agent->m_position.y +
//         innerCellPadding;
//
//     data.m_size = {agentSize, agentSize};
//
//     mFloat  death = 1.0f - a_agent->m_health / 100.0f;
//     ImVec4 colf;
//     if (a_agent->m_age < s_matureAge)
//     {
//         data.m_color = {death, 0.0f, 1.0f - death, 1.0f};
//     }
//     else
//     {
//         data.m_color = {death, 1.0f - death, 0.0f, 1.0f};
//     }
// }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void render_agent(AgentCharacter const* a_player)
{
    g_world.m_middleLayer.emplace_back();
    DrawData& data = g_world.m_middleLayer.back();

    static mFloat innerCellPadding = 3;
    data.m_position = {(parcelSize + parcelPadding) * a_player->m_position.x +
                           innerCellPadding,
                       (parcelSize + parcelPadding) * a_player->m_position.y +
                           innerCellPadding};

    data.m_size  = {heroSize, heroSize};
    data.m_color = {1.0f, 1.0f, 1.0f, 1.0f};
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// void render_agent(AgentMachine const* a_machine)
// {
//     g_world.m_frontLayer.emplace_back();
//     DrawData& data = g_world.m_frontLayer.back();
//
//     static mFloat innerCellPadding = -1;
//     static mFloat machineSize      = parcelSize - 2 * innerCellPadding;
//
//     data.m_position = {(parcelSize + parcelPadding) * a_machine->m_position.x
//     +
//                            innerCellPadding,
//                        (parcelSize + parcelPadding) * a_machine->m_position.y
//                        +
//                            innerCellPadding};
//
//     data.m_size   = {machineSize, machineSize};
//     data.m_color  = {0.5f, 0.5f, 0.5f, 1.0f};
//     data.m_filled = false;
//
//     g_world.m_frontLayer.emplace_back();
//     DrawData& dataSmall = g_world.m_frontLayer.back();
//
//     mFloat        midPoint  = machineSize / 2.0f;
//     static mFloat smallSize = 8.0f;
//
//     dataSmall.m_size  = {smallSize, smallSize};
//     dataSmall.m_color = {0.5f, 0.5f, 0.5f, 1.0f};
//
//     switch (a_machine->m_orientation)
//     {
//         case IOrientable::Up:
//         {
//             dataSmall.m_position = {
//                 data.m_position.x + midPoint - smallSize / 2.0f,
//                 data.m_position.y};
//         }
//         break;
//         case IOrientable::Down:
//         {
//             dataSmall.m_position = {
//                 data.m_position.x + midPoint - smallSize / 2.0f,
//                 data.m_position.y + machineSize - smallSize};
//         }
//         break;
//         case IOrientable::Left:
//         {
//             dataSmall.m_position = {
//                 data.m_position.x,
//                 data.m_position.y + midPoint - smallSize / 2.0f};
//         }
//         break;
//         case IOrientable::Right:
//         {
//             dataSmall.m_position = {
//                 data.m_position.x + machineSize - smallSize,
//                 data.m_position.y + midPoint - smallSize / 2.0f};
//         }
//         break;
//         default: break;
//     }
// }