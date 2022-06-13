#pragma once

#include "Scene.hpp"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class GameAction
{
   private:
    class IGameActionConcept
    {
       public:
        virtual void                execute()     = 0;
        virtual void* create_copy() = 0;
    };
    template <typename t_GameAction>
    class GameActionImpl : public IGameActionConcept
    {
       public:
        template <typename... t_Args>
        GameActionImpl(t_GameAction* a_data, t_Args... a_args)
        {
            m_data = a_data;
            m_data->initialize(a_args...);
        }
        ~GameActionImpl() { delete m_data; }

        virtual void execute() override { m_data->execute(); }
        virtual void* create_copy() override { return m_data->create_copy(); }

        t_GameAction* m_data;
    };

    IGameActionConcept* m_gameAction;

   public:
    template <typename t_GameAction, typename... t_Args>
    GameAction(t_GameAction* a_data, t_Args... a_args)
    {
        m_gameAction = new GameActionImpl<t_GameAction>(a_data, a_args...);
    }

    ~GameAction() { delete m_gameAction; }

    void execute() { m_gameAction->execute(); }
};

// Games Actions must be lightweight
// struct GameAction
//{
//    virtual ~GameAction()  = default;
//    virtual void execute() = 0;
//};

struct SpawnModelSomewhereGA
{
    void execute();

    ComponentManager&  targetCpntManager;
    TransformCpnt      spawnTransform;
    ModelBank::ModelID spawnModel;
};

struct KillEntityGA
{
    KillEntityGA() = delete;
    KillEntityGA(KillEntityGA const& a_ga);
    KillEntityGA(ComponentManager& a_cm, Entity a_e);

    void execute();

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
    template <typename... t_Args>
    void add_gameAction(GameAction const& a_GameAction, t_Args... a_args);

    void clear();
    void execute_gameActions();

   private:
    std::vector<GameAction*> m_actions;
};

template <typename t_GameAction>
void GameActionProcessor::add_gameAction(t_GameAction const& a_GameAction)
{
    m_actions.push_back(new GameAction(a_GameAction));
}

template <typename... t_Args>
void GameActionProcessor::add_gameAction(GameAction const& a_GameAction,
                                         t_Args... a_args)
{
}

template <typename t_GameAction, typename... t_Args>
void GameActionProcessor::add_gameAction(t_GameAction const& a_GameAction,
                                         t_Args... a_args)
{
    m_actions.push_back(new GameAction(a_GameAction, a_args...));
}

extern GameActionProcessor g_gameActionProcessor;