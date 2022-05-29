#include "GameAction.hpp"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void SpawnModelSomewhereGA::execute()
{
    targetCpntManager.create_entityFromModel(g_modelBank.models[spawnModel],
                                             spawnTransform);
}

KillEntityGA::KillEntityGA(KillEntityGA const& a_ga)
    : targetCpntManager(a_ga.targetCpntManager),
      entityToKill(a_ga.entityToKill)
{
}

KillEntityGA::KillEntityGA(ComponentManager& a_cm, Entity a_e)
    : targetCpntManager(a_cm),
      entityToKill(a_e)
{
}

void KillEntityGA::execute()
{
    targetCpntManager.kill_entity(entityToKill);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GameActionProcessor g_gameActionProcessor;

void GameActionProcessor::execute_gameActions()
{
    for (auto GA : m_actions)
    {
        GA->execute();
        delete GA;
    }
    m_actions.clear();
}

void GameActionProcessor::clear()
{
    for (auto GA : m_actions) { delete GA; }
    m_actions.clear();
}