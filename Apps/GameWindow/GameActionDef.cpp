#include "GameActionDef.hpp"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GASpawnModelSomewhere::execute(ComponentManager& a_cpntManager)
{
    //a_cpntManager.create_entityFromModel(g_modelBank.models[spawnModel],
    //                                     spawnTransform);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
GAKillEntity* GAKillEntity::create(GAKillEntity::InternalData& a_data)
{
    return new GAKillEntity(a_data);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void GAKillEntity::execute(ComponentManager& a_cpntManager)
{
    a_cpntManager.kill_entity(internalData.entityToKill);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
GASelfDestruct* GASelfDestruct::create(GASelfDestruct::InternalData& a_data)
{
    return new GASelfDestruct(a_data);
}