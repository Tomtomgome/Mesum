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
void GameActionDesc::read(std::ifstream& a_inputStream)
{
    m::mU32     version;
    std::string debugName;
    a_inputStream >> debugName >> version;

    m::mI32 tmpInt;
    a_inputStream >> tmpInt;
    type = static_cast<GameAction::Type>(tmpInt);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GameActionDesc::write(std::ofstream& a_outputStream) const
{
    a_outputStream << "GameActionDesc: " << s_version << ' ';

    a_outputStream << static_cast<m::mI32>(type) << std::endl;
}

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