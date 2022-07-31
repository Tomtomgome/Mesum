#include "GameAction.hpp"

#include <imgui.h>

const char* GameAction::GATypeNames[3] = {
    "Spawn Model",
    "Kill Entity",
    "Self Destruct",
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GameActionDesc::display_gui()
{
    const char* preview = GameAction::GATypeNames[type];

    if (ImGui::BeginCombo("Action type", preview))
    {
        for (m::mInt i = 0; i < m::mInt(GameAction::Type::_count); ++i)
        {
            auto iType = static_cast<GameAction::Type>(i);
            if (ImGui::Selectable(GameAction::GATypeNames[i], iType == type))
            {
                type = iType;
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
void execute_gameActions(std::vector<GameAction*>& a_gameActions,
                         ComponentManager&         a_cpntManager)
{
    for (auto action : a_gameActions)
    {
        action->execute(a_cpntManager);
        delete action;
    }
    a_gameActions.clear();
}