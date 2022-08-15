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
    void display_gui();

    GameAction::Type type;
};
mBegin_serialization(GameActionDesc, 1)

    m::mI32 gaType = static_cast<m::mI32>(a_object.type);
mSerialize_from(1, gaType);
a_object.type = static_cast<GameAction::Type>(gaType);

mEnd_serialization(GameActionDesc);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void execute_gameActions(std::vector<GameAction*>& a_gameActions,
                         ComponentManager&         a_cpntManager);