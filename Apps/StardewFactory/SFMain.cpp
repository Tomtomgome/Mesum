#include <MesumGraphics/DearImgui/imgui.h>

#include <MesumCore/Kernel/Kernel.hpp>
#include <MesumCore/Kernel/Math.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/WindowedApp.hpp>

using namespace m;

#define FIELD_SIZE 10

#define INVENTORY_SIZE 5

static const Float s_matureAge                   = 3.0f;
static const Float s_seedPrice                   = 150.0f;
static const Float s_plantDeathRateWhenHarvested = 5.0f;
static const Float s_plantDeathRateWhenGrounded  = 10.0f;
static const Float s_plantBaseConsumptionRate    = 1.0f;
static const Float s_plantBaseHealth             = 100.0f;

static const Float s_fieldRegenerationRate = 0.3f;
static const Float s_fieldMaxNutiments     = 10.0f;

static ImVec4 colField = {0.284, 0.159, 0.0f, 1.0f};

const static Float parcelSize    = 40;
const static Float parcelPadding = 3;

const static Float agentSizeSmall = 2;
const static Float agentSizeBig   = 10;

const static Float heroSize = 4;

static Float s_machineRefreshTime = 1.0f;

//*****************************************************************************
// Basic field
//*****************************************************************************
struct IAgent;

struct Field
{
    std::set<IAgent*> m_cells[FIELD_SIZE][FIELD_SIZE];
    Float             m_nutrients[FIELD_SIZE][FIELD_SIZE];
};

//*****************************************************************************
// Basic agent
//*****************************************************************************
enum AgentType
{
    Plant   = 0,
    Player  = 1,
    Machine = 2
};

struct IAgent
{
    IAgent(AgentType a_type)
    {
        m_type       = a_type;
        m_position.x = 0;
        m_position.y = 0;
    }
    virtual ~IAgent() {}

    virtual void update(Field& a_field, Double a_deltaTime) = 0;

    math::IVec2 m_position;
    AgentType   m_type;
};

class Agent
{
   private:
    class IAgentConcept
    {
       public:
        virtual void update(Field& a_field, Double a_deltaTime) = 0;
        virtual void display()                                  = 0;
    };
    template <typename T>
    class AgentImpl : public IAgentConcept
    {
        AgentImpl(T* a_data) { m_data = a_data; }
        ~AgentImpl() { delete m_data; }

        virtual void update(Field& a_field, Double a_deltaTime) override
        {
            m_data->update(a_field, a_deltaTime);
        }

        virtual void display() override { display(m_data); }

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

    void update(Field& a_field, Double a_deltaTime)
    {
        m_agent->update(a_field, a_deltaTime);
    }
    void display() { m_agent->display(); }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void place_agent(Field& a_field, IAgent* a_agent, Int a_x, Int a_y)
{
    a_agent->m_position.x = a_x;
    a_agent->m_position.y = a_y;

    a_field.m_cells[a_x][a_y].insert(a_agent);
}

//*****************************************************************************
// Crop agent
//*****************************************************************************
struct AgentPlant : public IAgent
{
    AgentPlant() : IAgent(AgentType::Plant) {}

    virtual void update(Field& a_field, Double a_deltaTime) override
    {
        if (m_isHarvested)
        {
            m_health = std::max(
                m_health - s_plantDeathRateWhenHarvested * a_deltaTime, 0.0);
            return;
        }

        m::Float& cell = a_field.m_nutrients[m_position.x][m_position.y];
        if (cell - m_consumption >= 0)
        {
            cell -= m_consumption * a_deltaTime;
            if (m_age < s_matureAge)
            {
                m_age += a_deltaTime;
            }
        }
        else
        {
            m_health = std::max(
                m_health - s_plantDeathRateWhenGrounded * a_deltaTime, 0.0);
        }
    }

    void harvest() { m_isHarvested = true; }

    Bool  m_isHarvested = false;
    Float m_consumption = s_plantBaseConsumptionRate;
    Float m_age         = 0.0f;
    Float m_health      = s_plantBaseHealth;
};

//*****************************************************************************
// Inventory basics
//*****************************************************************************

enum ObjectType
{
    Seed  = 0,
    Fruit = 1,
};

static std::string ObjectTypeNames[] = {"Seeds", "Fruits"};

struct Object
{
    ObjectType m_type;
    void*      m_data;
};

struct Slot
{
    std::vector<Object> m_objects;
};

struct Inventory
{
    std::map<ObjectType, Slot>::iterator m_selectedSlot;
    std::map<ObjectType, Slot>           m_slots;
};

struct IAgentWithInventory : public IAgent
{
    IAgentWithInventory(AgentType a_type) : IAgent(a_type) {}
    Inventory m_inventory;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void add_objectToInventory(Inventory& a_inventory, Object a_object)
{
    a_inventory.m_slots[a_object.m_type].m_objects.push_back(a_object);
}

//*****************************************************************************
// Character agent
//*****************************************************************************
struct AgentCharacter : public IAgentWithInventory
{
    AgentCharacter() : IAgentWithInventory(AgentType::Player) {}

    virtual void update(Field& a_field, Double a_deltaTime) override {}
    Float        m_money = 0.0f;
};

//*****************************************************************************
// Basic Command
//*****************************************************************************
struct ICommand
{
    virtual Bool execute() = 0;
};

//*****************************************************************************
// Positionable
//*****************************************************************************
struct IPositionable
{
    void rotation_clockwise()
    {
        m_orientation =
            static_cast<Orientation>((m_orientation + 1) % Orientation::_count);
    }

    void rotation_counterClockwise()
    {
        m_orientation = static_cast<Orientation>(
            (m_orientation + Orientation::_count - 1) % Orientation::_count);
    }

    Orientation m_orientation = Orientation::Up;
};

//*****************************************************************************
// Orientables
//*****************************************************************************
struct IOrientable
{
    enum Orientation
    {
        Up = 0,
        Right,
        Down,
        Left,
        _count
    };

    void rotation_clockwise()
    {
        m_orientation =
            static_cast<Orientation>((m_orientation + 1) % Orientation::_count);
    }

    void rotation_counterClockwise()
    {
        m_orientation = static_cast<Orientation>(
            (m_orientation + Orientation::_count - 1) % Orientation::_count);
    }

    Orientation m_orientation = Orientation::Up;
};

//*****************************************************************************
// Machines
//*****************************************************************************
struct AgentMachine : public IAgentWithInventory, public IOrientable
{
    AgentMachine() : IAgentWithInventory(AgentType::Machine) {}
    virtual ~AgentMachine()
    {
        for (auto command : m_instructions) { delete command; }
        m_instructions.clear();
    }

    virtual void update(Field& a_field, Double a_deltaTime) override
    {
        if (m_instructions.size() == 0)
        {
            return;
        }

        m_timeBeforeNextStep -= a_deltaTime;
        if (m_timeBeforeNextStep > 0)
        {
            return;
        }

        m_timeBeforeNextStep = s_machineRefreshTime;

        m_instructions[m_instructionToExecute]->execute();
        m_instructionToExecute =
            (m_instructionToExecute + 1U) % m_instructions.size();
    }

    Float                  m_timeBeforeNextStep   = s_machineRefreshTime;
    U64                    m_instructionToExecute = 0;
    std::vector<ICommand*> m_instructions;
};

//*****************************************************************************
// Movement related commands
//*****************************************************************************

struct CommandMoveUp : public ICommand
{
    virtual Bool execute() override
    {
        if (m_agentToMove->m_position.y <= 0)
        {
            return false;
        }

        std::set<IAgent*> cellContent =
            m_field->m_cells[m_agentToMove->m_position.x]
                            [m_agentToMove->m_position.y];
        auto agent = cellContent.find(m_agentToMove);
        mAssert(agent != cellContent.end());

        cellContent.erase(agent);

        m_agentToMove->m_position.y--;

        m_field
            ->m_cells[m_agentToMove->m_position.x][m_agentToMove->m_position.y]
            .insert(m_agentToMove);

        return true;
    }

    IAgent* m_agentToMove;
    Field*  m_field;
};

struct CommandMoveDown : public ICommand
{
    virtual Bool execute() override
    {
        if (m_agentToMove->m_position.y >= FIELD_SIZE - 1)
        {
            return false;
        }

        std::set<IAgent*> cellContent =
            m_field->m_cells[m_agentToMove->m_position.x]
                            [m_agentToMove->m_position.y];
        auto agent = cellContent.find(m_agentToMove);
        mAssert(agent != cellContent.end());

        cellContent.erase(agent);

        m_agentToMove->m_position.y++;

        m_field
            ->m_cells[m_agentToMove->m_position.x][m_agentToMove->m_position.y]
            .insert(m_agentToMove);

        return true;
    }

    IAgent* m_agentToMove;
    Field*  m_field;
};

struct CommandMoveLeft : public ICommand
{
    virtual Bool execute() override
    {
        if (m_agentToMove->m_position.x <= 0)
        {
            return false;
        }

        std::set<IAgent*> cellContent =
            m_field->m_cells[m_agentToMove->m_position.x]
                            [m_agentToMove->m_position.y];
        auto agent = cellContent.find(m_agentToMove);
        mAssert(agent != cellContent.end());

        cellContent.erase(agent);

        m_agentToMove->m_position.x--;

        m_field
            ->m_cells[m_agentToMove->m_position.x][m_agentToMove->m_position.y]
            .insert(m_agentToMove);

        return true;
    }

    IAgent* m_agentToMove;
    Field*  m_field;
};

struct CommandMoveRight : public ICommand
{
    virtual Bool execute() override
    {
        if (m_agentToMove->m_position.x >= FIELD_SIZE - 1)
        {
            return false;
        }

        std::set<IAgent*> cellContent =
            m_field->m_cells[m_agentToMove->m_position.x]
                            [m_agentToMove->m_position.y];
        auto agent = cellContent.find(m_agentToMove);
        mAssert(agent != cellContent.end());

        cellContent.erase(agent);

        m_agentToMove->m_position.x++;

        m_field
            ->m_cells[m_agentToMove->m_position.x][m_agentToMove->m_position.y]
            .insert(m_agentToMove);

        return true;
    }

    IAgent* m_agentToMove;
    Field*  m_field;
};

//*****************************************************************************
// Orientation related commands
//*****************************************************************************

struct CommandMoveForward : public ICommand
{
    virtual Bool execute() override
    {
        switch (m_orientable->m_orientation)
        {
            case IOrientable::Orientation::Up:
            {
                CommandMoveUp command;
                command.m_agentToMove = m_agentToMove;
                command.m_field       = m_field;
                return command.execute();
            }
            case IOrientable::Orientation::Right:
            {
                CommandMoveRight command;
                command.m_agentToMove = m_agentToMove;
                command.m_field       = m_field;
                return command.execute();
            }
            case IOrientable::Orientation::Down:
            {
                CommandMoveDown command;
                command.m_agentToMove = m_agentToMove;
                command.m_field       = m_field;
                return command.execute();
            }
            case IOrientable::Orientation::Left:
            {
                CommandMoveLeft command;
                command.m_agentToMove = m_agentToMove;
                command.m_field       = m_field;
                return command.execute();
            }
            default: return false;
        }
    }

    IOrientable* m_orientable;
    IAgent*      m_agentToMove;
    Field*       m_field;
};

struct CommandRotateClockWise : public ICommand
{
    virtual Bool execute() override
    {
        m_orientable->rotation_clockwise();
        return true;
    }

    IOrientable* m_orientable;
};

struct CommandRotateCounterClockWise : public ICommand
{
    virtual Bool execute() override
    {
        m_orientable->rotation_counterClockwise();
        return true;
    }

    IOrientable* m_orientable;
};

//*****************************************************************************
// Crop related commands
//*****************************************************************************
struct CommandPlantCrop : public ICommand
{
    virtual Bool execute() override
    {
        mAssert(m_agent != nullptr);
        mAssert(m_field != nullptr);
        IAgentWithInventory& agent = *m_agent;
        Field&               field = *m_field;

        std::set<IAgent*>& cell =
            field.m_cells[agent.m_position.x][agent.m_position.y];

        for (auto agent : cell)
        {
            if (agent->m_type == AgentType::Plant)
            {
                return false;
            }
        }

        auto seedSlot = agent.m_inventory.m_slots.find(ObjectType::Seed);

        if (seedSlot == agent.m_inventory.m_slots.end() ||
            seedSlot->second.m_objects.size() == 0)
        {
            return false;
        }

        IAgent* newAgent = new AgentPlant();
        m_agents->insert(newAgent);
        place_agent(field, newAgent, agent.m_position.x, agent.m_position.y);
        seedSlot->second.m_objects.erase(--seedSlot->second.m_objects.end());
        return true;
    }

    IAgentWithInventory* m_agent;
    Field*               m_field;
    std::set<IAgent*>*   m_agents;
};

struct CommandHarvestCrop : public ICommand
{
    virtual Bool execute() override
    {
        mAssert(m_agent != nullptr);
        mAssert(m_field != nullptr);
        IAgentWithInventory& agent = *m_agent;
        Field&               field = *m_field;

        std::set<IAgent*>& cell =
            field.m_cells[agent.m_position.x][agent.m_position.y];

        Bool                        hasAPlant  = false;
        std::set<IAgent*>::iterator plantAgent = cell.begin();
        while (plantAgent != cell.end())
        {
            if ((*plantAgent)->m_type == AgentType::Plant)
            {
                hasAPlant = true;
                break;
            }
            ++plantAgent;
        }

        if (!hasAPlant)
        {
            return false;
        }

        static_cast<AgentPlant*>(*plantAgent)->harvest();

        add_objectToInventory(agent.m_inventory,
                              {ObjectType::Fruit, *plantAgent});

        cell.erase(plantAgent);

        return true;
    }

    IAgentWithInventory* m_agent;
    Field*               m_field;
    std::set<IAgent*>*   m_agents;
};

//*****************************************************************************
// Basic Updates
//*****************************************************************************

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void update_field(Field& a_field, Double a_deltaTime)
{
    for (Int i = 0; i < FIELD_SIZE; ++i)
    {
        for (Int j = 0; j < FIELD_SIZE; ++j)
        {
            a_field.m_nutrients[i][j] = std::min(
                a_field.m_nutrients[i][j] +
                    s_fieldRegenerationRate * static_cast<Float>(a_deltaTime),
                s_fieldMaxNutiments);
        }
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// void update_agents(std::set<IAgent*>& a_agentsToUpdate, Field& a_field,
//                    Double a_deltaTime)
// {
//     for (auto& agent : a_agentsToUpdate)
//     {
//         agent->update(a_field, a_deltaTime);
//     }
// }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void update_agents(std::set<Agent*>& a_agentsToUpdate, Field& a_field,
                   Double a_deltaTime)
{
    for (auto agent : a_agentsToUpdate) { agent->update(a_field, a_deltaTime); }
}

//*****************************************************************************
// Displays
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void display_field(Field const& a_field)
{
    const ImVec2 p         = ImGui::GetCursorScreenPos();
    Float        cx        = p.x + 5.0f;
    Float        cy        = p.y + 5.0f;
    ImDrawList*  draw_list = ImGui::GetWindowDrawList();

    for (Int i = 0; i < FIELD_SIZE; ++i)
    {
        Float x = cx + (parcelSize + parcelPadding) * i;
        for (Int j = 0; j < FIELD_SIZE; ++j)
        {
            Float       y    = cy + (parcelSize + parcelPadding) * j;
            ImVec4      colf = ImVec4(colField.x, colField.y, colField.z,
                                 a_field.m_nutrients[i][j] / 10.0f);
            const ImU32 col  = ImColor(colf);

            draw_list->AddRectFilled(
                ImVec2(x, y), ImVec2(x + parcelSize, y + parcelSize), col);
        }
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// void display_plant(AgentPlant const& a_agent)
// {
//     if (a_agent.m_isHarvested)
//     {
//         return;
//     }
//
//     const ImVec2 p         = ImGui::GetCursorScreenPos();
//     Float        cx        = p.x + 5.0f;
//     Float        cy        = p.y + 5.0f;
//     ImDrawList*  draw_list = ImGui::GetWindowDrawList();
//
//     Float agentSize =
//         math::lerp(agentSizeSmall, agentSizeBig, a_agent.m_age /
//         s_matureAge);
//
//     Float innerCellPadding = parcelSize / 2.0f - agentSize / 2.0f;
//
//     Float x = cx + (parcelSize + parcelPadding) * a_agent.m_position.x +
//               innerCellPadding;
//     Float y = cy + (parcelSize + parcelPadding) * a_agent.m_position.y +
//               innerCellPadding;
//
//     Float death = 1.0f - a_agent.m_health / 100.0f;
//
//     ImVec4 colf;
//     if (a_agent.m_age < s_matureAge)
//     {
//         colf = ImVec4(death, 0.0f, 1.0f - death, 1.0f);
//     }
//     else
//     {
//         colf = ImVec4(death, 1.0f - death, 0.0f, 1.0f);
//     }
//     const ImU32 col = ImColor(colf);
//
//     draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + agentSize, y +
//     agentSize),
//                              col);
// }
//
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// void display_player(AgentCharacter& a_player)
// {
//     const ImVec2 p         = ImGui::GetCursorScreenPos();
//     Float        cx        = p.x + 5.0f;
//     Float        cy        = p.y + 5.0f;
//     ImDrawList*  draw_list = ImGui::GetWindowDrawList();
//
//     static Float innerCellPadding = 3;
//
//     Float x = cx + (parcelSize + parcelPadding) * a_player.m_position.x +
//               innerCellPadding;
//     Float y = cy + (parcelSize + parcelPadding) * a_player.m_position.y +
//               innerCellPadding;
//
//     ImVec4      colf = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
//     const ImU32 col  = ImColor(colf);
//
//     draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + heroSize, y +
//     heroSize),
//                              col);
// }
//
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// void display_machine(AgentMachine& a_machine)
// {
//     const ImVec2 p         = ImGui::GetCursorScreenPos();
//     Float        cx        = p.x + 5.0f;
//     Float        cy        = p.y + 5.0f;
//     ImDrawList*  draw_list = ImGui::GetWindowDrawList();
//
//     static Float innerCellPadding = -1;
//     static Float machineSize      = parcelSize - 2 * innerCellPadding;
//
//     Float x = cx + (parcelSize + parcelPadding) * a_machine.m_position.x +
//               innerCellPadding;
//     Float y = cy + (parcelSize + parcelPadding) * a_machine.m_position.y +
//               innerCellPadding;
//
//     ImVec4      colf = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
//     const ImU32 col  = ImColor(colf);
//
//     draw_list->AddRect(ImVec2(x, y), ImVec2(x + machineSize, y +
//     machineSize),
//                        col);
//
//     ImVec2       p1;
//     ImVec2       p2;
//     ImVec2       p3;
//     Float        midPoint     = machineSize / 2.0f;
//     static Float trinagleSize = 8.0f;
//     switch (a_machine.m_orientation)
//     {
//         case IOrientable::Up:
//         {
//             p1 = ImVec2(x + midPoint + trinagleSize / 2.0f, y);
//             p2 = ImVec2(x + midPoint - trinagleSize / 2.0f, y);
//             p3 = ImVec2(x + midPoint, y + trinagleSize);
//         }
//         break;
//         case IOrientable::Down:
//         {
//             p1 = ImVec2(x + midPoint - trinagleSize / 2.0f, y + machineSize);
//             p2 = ImVec2(x + midPoint + trinagleSize / 2.0f, y + machineSize);
//             p3 = ImVec2(x + midPoint, y + machineSize - trinagleSize);
//         }
//         break;
//         case IOrientable::Left:
//         {
//             p1 = ImVec2(x, y + midPoint - trinagleSize / 2.0f);
//             p2 = ImVec2(x, y + midPoint + trinagleSize / 2.0f);
//             p3 = ImVec2(x + trinagleSize, y + midPoint);
//         }
//         break;
//         case IOrientable::Right:
//         {
//             p1 = ImVec2(x + machineSize, y + midPoint - trinagleSize / 2.0f);
//             p2 = ImVec2(x + machineSize, y + midPoint + trinagleSize / 2.0f);
//             p3 = ImVec2(x + machineSize - trinagleSize, y + midPoint);
//         }
//         break;
//         default: break;
//     }
//     draw_list->AddTriangleFilled(p1, p2, p3, col);
// }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void display(AgentPlant const* a_agent)
{
    if (a_agent->m_isHarvested)
    {
        return;
    }

    const ImVec2 p         = ImGui::GetCursorScreenPos();
    Float        cx        = p.x + 5.0f;
    Float        cy        = p.y + 5.0f;
    ImDrawList*  draw_list = ImGui::GetWindowDrawList();

    Float agentSize =
        math::lerp(agentSizeSmall, agentSizeBig, a_agent->m_age / s_matureAge);

    Float innerCellPadding = parcelSize / 2.0f - agentSize / 2.0f;

    Float x = cx + (parcelSize + parcelPadding) * a_agent->m_position.x +
              innerCellPadding;
    Float y = cy + (parcelSize + parcelPadding) * a_agent->m_position.y +
              innerCellPadding;

    Float death = 1.0f - a_agent->m_health / 100.0f;

    ImVec4 colf;
    if (a_agent->m_age < s_matureAge)
    {
        colf = ImVec4(death, 0.0f, 1.0f - death, 1.0f);
    }
    else
    {
        colf = ImVec4(death, 1.0f - death, 0.0f, 1.0f);
    }
    const ImU32 col = ImColor(colf);

    draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + agentSize, y + agentSize),
                             col);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void display(AgentCharacter const* a_player)
{
    const ImVec2 p         = ImGui::GetCursorScreenPos();
    Float        cx        = p.x + 5.0f;
    Float        cy        = p.y + 5.0f;
    ImDrawList*  draw_list = ImGui::GetWindowDrawList();

    static Float innerCellPadding = 3;

    Float x = cx + (parcelSize + parcelPadding) * a_player->m_position.x +
              innerCellPadding;
    Float y = cy + (parcelSize + parcelPadding) * a_player->m_position.y +
              innerCellPadding;

    ImVec4      colf = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    const ImU32 col  = ImColor(colf);

    draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + heroSize, y + heroSize),
                             col);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void display(AgentMachine const* a_machine)
{
    const ImVec2 p         = ImGui::GetCursorScreenPos();
    Float        cx        = p.x + 5.0f;
    Float        cy        = p.y + 5.0f;
    ImDrawList*  draw_list = ImGui::GetWindowDrawList();

    static Float innerCellPadding = -1;
    static Float machineSize      = parcelSize - 2 * innerCellPadding;

    Float x = cx + (parcelSize + parcelPadding) * a_machine->m_position.x +
              innerCellPadding;
    Float y = cy + (parcelSize + parcelPadding) * a_machine->m_position.y +
              innerCellPadding;

    ImVec4      colf = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    const ImU32 col  = ImColor(colf);

    draw_list->AddRect(ImVec2(x, y), ImVec2(x + machineSize, y + machineSize),
                       col);

    ImVec2       p1;
    ImVec2       p2;
    ImVec2       p3;
    Float        midPoint     = machineSize / 2.0f;
    static Float trinagleSize = 8.0f;
    switch (a_machine->m_orientation)
    {
        case IOrientable::Up:
        {
            p1 = ImVec2(x + midPoint + trinagleSize / 2.0f, y);
            p2 = ImVec2(x + midPoint - trinagleSize / 2.0f, y);
            p3 = ImVec2(x + midPoint, y + trinagleSize);
        }
        break;
        case IOrientable::Down:
        {
            p1 = ImVec2(x + midPoint - trinagleSize / 2.0f, y + machineSize);
            p2 = ImVec2(x + midPoint + trinagleSize / 2.0f, y + machineSize);
            p3 = ImVec2(x + midPoint, y + machineSize - trinagleSize);
        }
        break;
        case IOrientable::Left:
        {
            p1 = ImVec2(x, y + midPoint - trinagleSize / 2.0f);
            p2 = ImVec2(x, y + midPoint + trinagleSize / 2.0f);
            p3 = ImVec2(x + trinagleSize, y + midPoint);
        }
        break;
        case IOrientable::Right:
        {
            p1 = ImVec2(x + machineSize, y + midPoint - trinagleSize / 2.0f);
            p2 = ImVec2(x + machineSize, y + midPoint + trinagleSize / 2.0f);
            p3 = ImVec2(x + machineSize - trinagleSize, y + midPoint);
        }
        break;
        default: break;
    }
    draw_list->AddTriangleFilled(p1, p2, p3, col);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// void display_agents(std::set<IAgent*> const& a_agents)
// {
//     const ImVec2 p         = ImGui::GetCursorScreenPos();
//     Float        cx        = p.x + 5.0f;
//     Float        cy        = p.y + 5.0f;
//     ImDrawList*  draw_list = ImGui::GetWindowDrawList();
//
//     for (auto agent : a_agents)
//     {
//         switch (agent->m_type)
//         {
//             case AgentType::Plant:
//             {
//                 display_plant(*static_cast<AgentPlant*>(agent));
//             }
//             break;
//             case AgentType::Player:
//             {
//                 display_player(*static_cast<AgentCharacter*>(agent));
//             }
//             break;
//             case AgentType::Machine:
//             {
//                 display_machine(*static_cast<AgentMachine*>(agent));
//             }
//             break;
//             default: break;
//         }
//     }
// }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void display_agents(std::set<Agent*> const& a_agents)
{
    for (auto agent : a_agents) { agent->display(); }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void display_heroInventory(Inventory& a_inventory)
{
    for (auto slot = a_inventory.m_slots.begin();
         slot != a_inventory.m_slots.end(); ++slot)
    {
        std::stringstream slotName;
        slotName << ObjectTypeNames[slot->first] << " : "
                 << slot->second.m_objects.size();
        if (slot == a_inventory.m_selectedSlot)
        {
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  ImVec4(1.0f, 0.9f, 1.0f, 1.0f));
            ImGui::Text("%s", slotName.str().c_str());
            ImGui::PopStyleColor();
        }
        else
        {
            if (ImGui::Button(slotName.str().c_str()))
            {
                a_inventory.m_selectedSlot = slot;
            }
        }
    }
}

//*****************************************************************************
// App
//*****************************************************************************
class StardewFactoryApp : public m::crossPlatform::IWindowedApplication
{
    void move_up()
    {
        CommandMoveUp command;
        command.m_agentToMove = m_player;
        command.m_field       = &m_field;
        command.execute();
    }
    void move_down()
    {
        CommandMoveDown command;
        command.m_agentToMove = m_player;
        command.m_field       = &m_field;
        command.execute();
    }
    void move_left()
    {
        CommandMoveLeft command;
        command.m_agentToMove = m_player;
        command.m_field       = &m_field;
        command.execute();
    }
    void move_right()
    {
        CommandMoveRight command;
        command.m_agentToMove = m_player;
        command.m_field       = &m_field;
        command.execute();
    }

    void player_action()
    {
        if (m_player->m_inventory.m_selectedSlot ==
            m_player->m_inventory.m_slots.end())
        {
            CommandHarvestCrop commandHarvest;
            commandHarvest.m_agent  = m_player;
            commandHarvest.m_field  = &m_field;
            commandHarvest.m_agents = &m_agents;
            commandHarvest.execute();
            return;
        }

        switch (m_player->m_inventory.m_selectedSlot->first)
        {
            case ObjectType::Seed:
            {
                CommandPlantCrop commandPlant;
                commandPlant.m_agent  = m_player;
                commandPlant.m_field  = &m_field;
                commandPlant.m_agents = &m_agents;

                if (!commandPlant.execute())
                {
                    CommandHarvestCrop commandHarvest;
                    commandHarvest.m_agent  = m_player;
                    commandHarvest.m_field  = &m_field;
                    commandHarvest.m_agents = &m_agents;
                    commandHarvest.execute();
                }
            }
            break;
            case ObjectType::Fruit:
            {
                CommandHarvestCrop commandHarvest;
                commandHarvest.m_agent  = m_player;
                commandHarvest.m_field  = &m_field;
                commandHarvest.m_agents = &m_agents;
                commandHarvest.execute();
            }
            break;
            default:
            {
                CommandHarvestCrop commandHarvest;
                commandHarvest.m_agent  = m_player;
                commandHarvest.m_field  = &m_field;
                commandHarvest.m_agents = &m_agents;
                commandHarvest.execute();
            }
            break;
        }
    }

    void player_buySeed()
    {
        if (m_player->m_money >= s_seedPrice)
        {
            m_player->m_money -= s_seedPrice;
            add_objectToInventory(m_player->m_inventory, {ObjectType::Seed});
        }
    }

    void player_sellFruits()
    {
        auto fruits = m_player->m_inventory.m_slots.find(ObjectType::Fruit);
        if (fruits != m_player->m_inventory.m_slots.end())
        {
            for (auto fruit : fruits->second.m_objects)
            {
                AgentPlant* fruitAgent = static_cast<AgentPlant*>(fruit.m_data);
                m_player->m_money += fruitAgent->m_age * fruitAgent->m_health;
                m_agents.erase(fruitAgent);
                delete fruitAgent;
            }
            fruits->second.m_objects.clear();
        }
    }

    virtual void init() override
    {
        m::crossPlatform::IWindowedApplication::init();

        init_renderer(m::render::RendererApi::Vulkan);

        m_mainWindow = add_newWindow(L"Stardew Factory", 1280, 720);
        m_mainWindow->set_asMainWindow();

        m::Bool MultiViewportsEnabled = false;
        m_mainWindow->set_asImGuiWindow(MultiViewportsEnabled);
        set_processImGuiMultiViewports(MultiViewportsEnabled);

        m_mainWindow->link_inputManager(&m_inputManager);

        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyPressed(m::input::KEY_F11),
            m::input::KeyActionCallback(
                m_mainWindow, &m::windows::IWindow::toggle_fullScreen));

        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyPressed(m::input::KEY_UP),
            m::input::KeyActionCallback(this, &StardewFactoryApp::move_up));
        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyPressed(m::input::KEY_DOWN),
            m::input::KeyActionCallback(this, &StardewFactoryApp::move_down));
        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyPressed(m::input::KEY_LEFT),
            m::input::KeyActionCallback(this, &StardewFactoryApp::move_left));
        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyPressed(m::input::KEY_RIGHT),
            m::input::KeyActionCallback(this, &StardewFactoryApp::move_right));

        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyPressed(m::input::KEY_SPACE),
            m::input::KeyActionCallback(this,
                                        &StardewFactoryApp::player_action));

        set_microSecondsLimit(16000);

        // Initialize Field
        for (Int i = 0; i < FIELD_SIZE; ++i)
        {
            for (Int j = 0; j < FIELD_SIZE; ++j)
            {
                m_field.m_nutrients[i][j] = s_fieldMaxNutiments;
            }
        }

        // Initialize Player

        AgentCharacter* m_player = new AgentCharacter();
        m_player->m_position.x   = 0;
        m_player->m_position.y   = 0;
        m_player->m_money        = 0.0f;
        for (Int i = 0; i < 10; ++i)
        {
            add_objectToInventory(m_player->m_inventory,
                                  {ObjectType::Seed, nullptr});
        }
        m_player->m_inventory.m_selectedSlot =
            m_player->m_inventory.m_slots.end();

        place_agent(m_field, m_player, 0, 0);

        m_newPlayer = new Agent(m_player);
        m_newAgents.insert(m_newPlayer);

        // Initialize Machine
        AgentMachine* m_machine = new AgentMachine();
        for (Int i = 0; i < 100; ++i)
        {
            add_objectToInventory(m_machine->m_inventory,
                                  {ObjectType::Seed, nullptr});
        }
        m_machine->m_orientation       = IOrientable::Right;
        CommandPlantCrop* commandPlant = new CommandPlantCrop();
        commandPlant->m_agent          = m_machine;
        commandPlant->m_field          = &m_field;
        commandPlant->m_agents         = &m_agents;
        m_machine->m_instructions.push_back(commandPlant);
        CommandMoveForward* command = new CommandMoveForward();
        command->m_orientable       = m_machine;
        command->m_agentToMove      = m_machine;
        command->m_field            = &m_field;
        m_machine->m_instructions.push_back(command);
        place_agent(m_field, m_machine, 0, 0);

        m_newMachine = new Agent(m_machine);
        m_newAgents.insert(m_newMachine);
    }

    virtual void destroy() override
    {
        for (auto agent : m_newAgents) { delete agent; }
        //         m_agents.clear();

        m_newAgents.clear();

        m::crossPlatform::IWindowedApplication::destroy();
        // Nothing to destroy
    }

    virtual m::Bool step(const m::Double& a_deltaTime) override
    {
        if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
        {
            return false;
        }

        static Double s_time = 0;
        s_time += a_deltaTime;

        m_inputManager.processAndUpdate_States();

        update_field(m_field, a_deltaTime);
        update_agents(m_newAgents, m_field, a_deltaTime);

        start_dearImGuiNewFrame();

        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        ImGui::Begin("Engine");
        {
            ImGui::ColorEdit4("Color", &colField.x, ImGuiColorEditFlags_Float);
            ImGui::Text("frame time : %f", a_deltaTime);
            ImGui::Text("total time : %f", s_time);
            ImGui::DragFloat("Machine refresh time", &s_machineRefreshTime,
                             0.01f, 0.0f);
            if (ImGui::Button("Turn clock"))
            {
                m_machine->rotation_clockwise();
            }
        }
        ImGui::End();

        ImGui::Begin("Inventory");
        {
            display_heroInventory(m_player->m_inventory);
        }
        ImGui::End();

        ImGui::Begin("Stats");
        {
            ImGui::Text("Money : %f", m_player->m_money);

            if (ImGui::Button("Buy seed"))
            {
                player_buySeed();
            }
            if (ImGui::Button("Sell Fruits"))
            {
                player_sellFruits();
            }
        }
        ImGui::End();

        ImGui::Begin("Field window");
        {
            display_field(m_field);
            display_agents(m_newAgents);
        }
        ImGui::End();

        ImGui::Render();

        render();
        return true;
    }

    Field m_field;
    // AgentCharacter*   m_player;
    // AgentMachine* m_machine;
    // std::set<IAgent*> m_agents;
    Agent*           m_newPlayer;
    Agent*           m_newMachine;
    std::set<Agent*> m_newAgents;

    m::input::InputManager m_inputManager;
    m::windows::IWindow*   m_mainWindow;

    const m::logging::ChannelID m_SFLOG_ID = mLOG_GET_ID();
};

M_EXECUTE_WINDOWED_APP(StardewFactoryApp)
