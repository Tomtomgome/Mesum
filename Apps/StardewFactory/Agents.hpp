#ifndef Agents_hpp
#define Agents_hpp
#pragma once

#include <MesumCore/Kernel/MathTypes.hpp>
#include <MesumCore/Kernel/Types.hpp>
#include <Traits.hpp>
#include <set>

//*****************************************************************************
// Basic agent
//*****************************************************************************
class Agent
{
   private:
    class IAgentConcept
    {
       public:
        virtual void update(m::mDouble a_deltaTime) = 0;
        virtual void render()                      = 0;
        virtual bool request_deletion()            = 0;
    };
    template <typename T>
    class AgentImpl : public IAgentConcept
    {
       public:
        AgentImpl(T* a_data) { m_data = a_data; }
        ~AgentImpl() { delete m_data; }

        virtual void update(m::mDouble a_deltaTime) override
        {
            m_data->update(a_deltaTime);
        }

        virtual void render() override { render_agent(m_data); }

        virtual m::mBool request_deletion() override
        {
            return m_data->request_deletion();
        }

        T* m_data;
    };

    IAgentConcept* m_agent;

   public:
    template <typename T>
    Agent(T* a_data)
    {
        m_agent = new AgentImpl<T>(a_data);
    }
    ~Agent() { delete m_agent; }

    void    update(m::mDouble a_deltaTime) { m_agent->update(a_deltaTime); }
    void    render() { m_agent->render(); }
    m::mBool request_deletion() { return m_agent->request_deletion(); }
};

struct AgentManager
{
    template <typename T>
    void add_agent(T* a_agentToAdd)
    {
        Agent* newAgent = new Agent(a_agentToAdd);
        m_agents.insert(newAgent);
    }

    void update_agents(m::mDouble a_deltaTime)
    {
        for (auto agent : m_agents)
        {
            if (agent->request_deletion())
            {
                m_agents.erase(agent);
                return;
            }
            agent->update(a_deltaTime);
        }
    }

    void render_agents()
    {
        for (auto agent : m_agents) { agent->render(); }
    }

    void clear()
    {
        for (auto agent : m_agents) { delete agent; }
        m_agents.clear();
    }

    std::set<Agent*> m_agents;
};

static const mFloat s_maxSoilNutrient              = 10.0f;
static const mFloat s_grassSoilRegenerationRate    = 0.1f;
static const mFloat s_wetDirtSoilRegenerationRate  = 0.05f;
static const mFloat s_dirtSoilRegenerationRate     = 0.03f;
static const mFloat s_grassToDirPropagationRate    = 30.0f;
static const mFloat s_grassToWetDirPropagationRate = 20.0f;

struct AgentSoil : public IPositionable, public IPermanent
{
    void update(mDouble a_deltaTime)
    {
        switch (m_type)
        {
            case AgentSoil::Grass:
            {
                m_nutrientQuantity +=
                    std::min(s_grassSoilRegenerationRate *
                                 static_cast<mFloat>(a_deltaTime),
                             s_maxSoilNutrient);
            }
            break;
            case AgentSoil::Dirt:
            {
                m_nutrientQuantity +=
                    std::min(s_dirtSoilRegenerationRate *
                                 static_cast<mFloat>(a_deltaTime),
                             s_maxSoilNutrient);
                update_grassState(a_deltaTime, s_grassToDirPropagationRate);
            }
            break;
            case AgentSoil::WetDirt:
            {
                m_nutrientQuantity +=
                    std::min(s_dirtSoilRegenerationRate *
                                 static_cast<mFloat>(a_deltaTime),
                             s_maxSoilNutrient);
                update_grassState(a_deltaTime, s_grassToWetDirPropagationRate);
            }
            break;
            default: break;
        }
    }

    void update_grassStateFromNeighbor(math::mIVec2 a_neighbor,
                                       mDouble      a_deltaTime,
                                       mFloat       a_propagationRate)
    {
        if (m_position.x + a_neighbor.x < 0 ||
            m_position.x + a_neighbor.x >= FIELD_SIZE)
            return;
        if (m_position.y + a_neighbor.y < 0 ||
            m_position.y + a_neighbor.y >= FIELD_SIZE)
            return;
        AgentSoil* neighbor =
            static_cast<AgentSoil*>(*(m_fieldOfSoil
                                          ->m_cells[m_position.x + a_neighbor.x]
                                                   [m_position.y + a_neighbor.y]
                                          .begin()));
        if (neighbor->m_type == Grass)
        {
            if (a_deltaTime / a_propagationRate >
                g_numberGenerator.get_nextDouble())
            {
                m_type = Grass;
            }
        }
    }

    void update_grassState(mDouble a_deltaTime, mFloat a_propagationRate)
    {
        if (!m_isOccupied)
        {
            update_grassStateFromNeighbor(math::mIVec2({-1, 0}), a_deltaTime,
                                          a_propagationRate);
            update_grassStateFromNeighbor(math::mIVec2({1, 0}), a_deltaTime,
                                          a_propagationRate);
            update_grassStateFromNeighbor(math::mIVec2({0, -1}), a_deltaTime,
                                          a_propagationRate);
            update_grassStateFromNeighbor(math::mIVec2({0, 1}), a_deltaTime,
                                          a_propagationRate);
        }
    }

    enum Type
    {
        Grass,
        Dirt,
        WetDirt,
    };

    Field* m_fieldOfSoil;
    Type   m_type;
    mBool  m_isOccupied;
    mFloat m_nutrientQuantity;
};

//*****************************************************************************
// Item
//*****************************************************************************
struct ItemHoe : public IItem
{
    virtual void use() override
    {
        AgentSoil* soil = static_cast<AgentSoil*>(
            *(m_fieldOfSoil
                  ->m_cells[m_positionable->m_position.x]
                           [m_positionable->m_position.y]
                  .begin()));

        if (soil->m_type == AgentSoil::Grass)
        {
            soil->m_type = AgentSoil::Dirt;
        }
    }

    virtual std::string get_name() override { return "Hoe"; }
    virtual mU64        get_id() override { return 1; }

    Field*         m_fieldOfSoil;
    IPositionable* m_positionable;
};

struct ItemWateringCan : public IItem
{
    virtual void use() override
    {
        AgentSoil* soil = static_cast<AgentSoil*>(
            *(m_fieldOfSoil
                  ->m_cells[m_positionable->m_position.x]
                           [m_positionable->m_position.y]
                  .begin()));

        if (soil->m_type == AgentSoil::Dirt)
        {
            soil->m_type = AgentSoil::WetDirt;
        }
    }

    virtual std::string get_name() override { return "Watering can"; }
    virtual mU64        get_id() override { return 2; }

    Field*         m_fieldOfSoil;
    IPositionable* m_positionable;
};

//*****************************************************************************
// Character agent
//*****************************************************************************
struct AgentCharacter : public IPositionable,
                        public IInventory,
                        public IPermanent
{
    void update(mDouble a_deltaTime) {}

    mFloat m_money = 0.0f;
};

#endif Agents_hpp