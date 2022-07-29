#pragma once

#include "Serializable.hpp"

#include <vector>

struct ComponentManager;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
struct GameAction
{
    enum Type
    {
        spawnModel = 0,
        killEntity,
        selfDestruct,

        _count
    };
    static const char* GATypeNames[3];

    virtual ~GameAction()                                 = default;
    virtual void execute(ComponentManager& a_cpntManager) = 0;
    virtual Type get_type()                               = 0;
};

struct GameActionDesc
{
    Serializable(0, GameActionDesc);
    void display_gui();

    GameAction::Type type;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void execute_gameActions(std::vector<GameAction*>& a_gameActions,
                         ComponentManager&         a_cpntManager);