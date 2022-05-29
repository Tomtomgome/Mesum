#pragma once

#include "Scene.hpp"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Games Actions must be lightweight
struct GameAction
{
    virtual ~GameAction()  = default;
    virtual void execute() = 0;
};

struct SpawnModelSomewhereGA : public GameAction
{
    void execute() override;

    ComponentManager&  targetCpntManager;
    TransformCpnt      spawnTransform;
    ModelBank::ModelID spawnModel;
};

struct KillEntityGA : public GameAction
{
    KillEntityGA() = delete;
    KillEntityGA(KillEntityGA const& a_ga);
    KillEntityGA(ComponentManager& a_cm, Entity a_e);

    void execute() override;

    ComponentManager& targetCpntManager;
    Entity            entityToKill;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class GameActionProcessor
{
   public:
    template <typename t_GameAction>
    void add_gameAction(t_GameAction const& a_GameAction);

    void clear();
    void execute_gameActions();

   private:
    std::vector<GameAction*> m_actions;
};

template <typename t_GameAction>
void GameActionProcessor::add_gameAction(t_GameAction const& a_GameAction)
{
    m_actions.push_back(new t_GameAction(a_GameAction));
}

extern GameActionProcessor g_gameActionProcessor;