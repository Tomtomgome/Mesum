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
// Basic agent
//*****************************************************************************
class Agent
{
   private:
    class IAgentConcept
    {
       public:
        virtual void update(Double a_deltaTime) = 0;
        virtual void display()                  = 0;
    };
    template <typename T>
    class AgentImpl : public IAgentConcept
    {
       public:
        AgentImpl(T* a_data) { m_data = a_data; }
        ~AgentImpl() { delete m_data; }

        virtual void update(Double a_deltaTime) override
        {
            m_data->update(a_deltaTime);
        }

        virtual void display() override { display_agent(m_data); }

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

    void update(Double a_deltaTime) { m_agent->update(a_deltaTime); }
    void display() { m_agent->display(); }
};

struct AgentManager
{
    template <typename T>
    void add_agent(T* a_agentToAdd)
    {
        Agent* newAgent = new Agent(a_agentToAdd);
        m_agents.insert(newAgent);
    }

    void update_agents(Double a_deltaTime)
    {
        for (auto agent : m_agents) { agent->update(a_deltaTime); }
    }

    void display_agents()
    {
        for (auto agent : m_agents) { agent->display(); }
    }

    void clear()
    {
        for (auto agent : m_agents) { delete agent; }
        m_agents.clear();
    }

    std::set<Agent*> m_agents;
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
    math::IVec2 m_position;
};

struct CommandMoveUp : public ICommand
{
    virtual Bool execute() override
    {
        if (m_positionableToMove->m_position.y <= 0)
        {
            return false;
        }

        m_positionableToMove->m_position.y--;
        return true;
    }

    IPositionable* m_positionableToMove;
};

struct CommandMoveDown : public ICommand
{
    virtual Bool execute() override
    {
        if (m_positionableToMove->m_position.y >= FIELD_SIZE - 1)
        {
            return false;
        }

        m_positionableToMove->m_position.y++;
        return true;
    }

    IPositionable* m_positionableToMove;
};

struct CommandMoveLeft : public ICommand
{
    virtual Bool execute() override
    {
        if (m_positionableToMove->m_position.x <= 0)
        {
            return false;
        }

        m_positionableToMove->m_position.x--;
        return true;
    }

    IPositionable* m_positionableToMove;
};

struct CommandMoveRight : public ICommand
{
    virtual Bool execute() override
    {
        if (m_positionableToMove->m_position.x >= FIELD_SIZE - 1)
        {
            return false;
        }

        m_positionableToMove->m_position.x++;
        return true;
    }

    IPositionable* m_positionableToMove;
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

struct CommandMoveForward : public ICommand
{
    virtual Bool execute() override
    {
        switch (m_orientable->m_orientation)
        {
            case IOrientable::Orientation::Up:
            {
                CommandMoveUp command;
                command.m_positionableToMove = m_positionableToMove;
                return command.execute();
            }
            case IOrientable::Orientation::Right:
            {
                CommandMoveRight command;
                command.m_positionableToMove = m_positionableToMove;
                return command.execute();
            }
            case IOrientable::Orientation::Down:
            {
                CommandMoveDown command;
                command.m_positionableToMove = m_positionableToMove;
                return command.execute();
            }
            case IOrientable::Orientation::Left:
            {
                CommandMoveLeft command;
                command.m_positionableToMove = m_positionableToMove;
                return command.execute();
            }
            default: return false;
        }
    }

    IOrientable*   m_orientable;
    IPositionable* m_positionableToMove;
};

//*****************************************************************************
// Fields
//*****************************************************************************
struct Field
{
    using Cell = std::set<IPositionable*>;
    Cell m_cells[FIELD_SIZE][FIELD_SIZE];
};

struct CommandPlaceOnField : public ICommand
{
    virtual Bool execute() override
    {
        Int x = m_positionableToPlace->m_position.x;
        Int y = m_positionableToPlace->m_position.y;
        m_field->m_cells[x][y].insert(m_positionableToPlace);

        return true;
    }

    IPositionable* m_positionableToPlace;
    Field*         m_field;
};

struct CommandRemoveFromField : public ICommand
{
    virtual Bool execute() override
    {
        Int x = m_positionableToRemoveMove->m_position.x;
        Int y = m_positionableToRemoveMove->m_position.y;
        m_field->m_cells[x][y].erase(m_positionableToRemoveMove);

        return true;
    }

    IPositionable* m_positionableToRemoveMove;
    Field*         m_field;
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

struct IInventory
{
    std::map<ObjectType, Slot>::iterator m_selectedSlot;
    std::map<ObjectType, Slot>           m_slots;
};

struct CommandAddObjectToInventory : ICommand
{
    Bool execute()
    {
        m_inventory->m_slots[m_objectToAdd.m_type].m_objects.push_back(
            m_objectToAdd);
        return true;
    }

    IInventory* m_inventory;
    Object      m_objectToAdd;
};

//*****************************************************************************
// Crops
//*****************************************************************************
struct AgentNutriment : public IPositionable
{
    void update(Double a_deltaTime)
    {
        m_nutrientQuantity +=
            std::min(s_fieldRegenerationRate * static_cast<Float>(a_deltaTime),
                     s_fieldMaxNutiments);
    }

    Float m_nutrientQuantity = s_fieldMaxNutiments;
};

struct AgentPlant : public IPositionable
{
    void update(Double a_deltaTime)
    {
        if (m_isHarvested)
        {
            m_health = std::max(
                m_health - s_plantDeathRateWhenHarvested * a_deltaTime, 0.0);
            return;
        }

        Field::Cell& cellContent =
            m_fieldOfNutriment->m_cells[m_position.x][m_position.y];
        AgentNutriment* nutriment =
            static_cast<AgentNutriment*>(*cellContent.begin());

        if (nutriment->m_nutrientQuantity - m_consumption >= 0)
        {
            nutriment->m_nutrientQuantity -= m_consumption * a_deltaTime;
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

    Field* m_fieldOfNutriment = nullptr;
    Bool   m_isHarvested      = false;
    Float  m_consumption      = s_plantBaseConsumptionRate;
    Float  m_age              = 0.0f;
    Float  m_health           = s_plantBaseHealth;
};

//*****************************************************************************
// Crop related commands
//*****************************************************************************
struct CommandPlantCrop : public ICommand
{
    virtual Bool execute() override
    {
        // Look if there is already a plant
        Field::Cell& cellContent =
            m_fieldOfPlants->m_cells[m_position->x][m_position->y];

        // Look if there is already a plant
        if (cellContent.begin() != cellContent.end())
        {
            return false;
        }

        auto seedSlot =
            m_inventoryToTakeSeedFrom->m_slots.find(ObjectType::Seed);

        if (seedSlot == m_inventoryToTakeSeedFrom->m_slots.end() ||
            seedSlot->second.m_objects.size() == 0)
        {
            return false;
        }

        // create a plant agent
        AgentPlant* newPlant = new AgentPlant();
        m_agentManager->add_agent(newPlant);

        newPlant->m_fieldOfNutriment = m_fieldOfNutriments;
        newPlant->m_position         = *m_position;

        CommandPlaceOnField placeOnField;
        placeOnField.m_field               = m_fieldOfPlants;
        placeOnField.m_positionableToPlace = newPlant;
        placeOnField.execute();

        seedSlot->second.m_objects.erase(--seedSlot->second.m_objects.end());
        return true;
    }

    AgentManager* m_agentManager;
    Field*        m_fieldOfPlants;
    Field*        m_fieldOfNutriments;
    IInventory*   m_inventoryToTakeSeedFrom;
    math::IVec2*  m_position;
};

struct CommandHarvestCrop : public ICommand
{
    virtual Bool execute() override
    {
        // Look if there is allready a plant
        Field::Cell& cellContent =
            m_fieldOfPlants->m_cells[m_position->x][m_position->y];

        // Look if there is no plant
        if (cellContent.begin() == cellContent.end())
        {
            return false;
        }

        AgentPlant* plantToHarvest =
            static_cast<AgentPlant*>(*cellContent.begin());
        // Harvest the plant
        plantToHarvest->harvest();

        CommandAddObjectToInventory commandAddObject;
        commandAddObject.m_inventory   = m_inventoryToAddFruitTo;
        commandAddObject.m_objectToAdd = {ObjectType::Fruit, plantToHarvest};
        commandAddObject.execute();

        cellContent.erase(plantToHarvest);

        return true;
    }

    Field*       m_fieldOfPlants;
    IInventory*  m_inventoryToAddFruitTo;
    math::IVec2* m_position;
};

//*****************************************************************************
// Character agent
//*****************************************************************************
struct AgentCharacter : public IPositionable, public IInventory
{
    void update(Double a_deltaTime) {}

    Float m_money = 0.0f;
};

//*****************************************************************************
// Machines
//*****************************************************************************
struct AgentMachine : public IPositionable,
                      public IInventory,
                      public IOrientable
{
    virtual ~AgentMachine()
    {
        for (auto command : m_instructions) { delete command; }
        m_instructions.clear();
    }

    void update(Double a_deltaTime)
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
// Displays
//*****************************************************************************
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void display_agent(AgentNutriment* a_nutriment)
{
    const ImVec2 p         = ImGui::GetCursorScreenPos();
    Float        cx        = p.x + 5.0f;
    Float        cy        = p.y + 5.0f;
    ImDrawList*  draw_list = ImGui::GetWindowDrawList();

    Float  x    = cx + (parcelSize + parcelPadding) * a_nutriment->m_position.x;
    Float  y    = cy + (parcelSize + parcelPadding) * a_nutriment->m_position.y;
    ImVec4 colf = ImVec4(colField.x, colField.y, colField.z,
                         a_nutriment->m_nutrientQuantity / s_fieldMaxNutiments);
    const ImU32 col = ImColor(colf);

    draw_list->AddRectFilled(ImVec2(x, y),
                             ImVec2(x + parcelSize, y + parcelSize), col);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void display_agent(AgentPlant const* a_agent)
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
void display_agent(AgentCharacter const* a_player)
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
void display_agent(AgentMachine const* a_machine)
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
void display_heroInventory(IInventory* a_inventory)
{
    for (auto slot = a_inventory->m_slots.begin();
         slot != a_inventory->m_slots.end(); ++slot)
    {
        std::stringstream slotName;
        slotName << ObjectTypeNames[slot->first] << " : "
                 << slot->second.m_objects.size();
        if (slot == a_inventory->m_selectedSlot)
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
                a_inventory->m_selectedSlot = slot;
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
        command.m_positionableToMove = m_player;
        command.execute();
    }
    void move_down()
    {
        CommandMoveDown command;
        command.m_positionableToMove = m_player;
        command.execute();
    }
    void move_left()
    {
        CommandMoveLeft command;
        command.m_positionableToMove = m_player;
        command.execute();
    }
    void move_right()
    {
        CommandMoveRight command;
        command.m_positionableToMove = m_player;
        command.execute();
    }

    void player_action()
    {
        if (m_player->m_selectedSlot == m_player->m_slots.end())
        {
            CommandHarvestCrop commandHarvest;
            commandHarvest.m_inventoryToAddFruitTo = m_player;
            commandHarvest.m_fieldOfPlants         = &m_fieldOfPlants;
            commandHarvest.m_position              = &m_player->m_position;
            commandHarvest.execute();
            return;
        }

        switch (m_player->m_selectedSlot->first)
        {
            case ObjectType::Seed:
            {
                CommandPlantCrop commandPlant;
                commandPlant.m_agentManager            = &m_agentManager;
                commandPlant.m_fieldOfPlants           = &m_fieldOfPlants;
                commandPlant.m_fieldOfNutriments       = &m_fieldOfNutriments;
                commandPlant.m_inventoryToTakeSeedFrom = m_player;
                commandPlant.m_position                = &m_player->m_position;
                if (!commandPlant.execute())
                {
                    CommandHarvestCrop commandHarvest;
                    commandHarvest.m_inventoryToAddFruitTo = m_player;
                    commandHarvest.m_fieldOfPlants         = &m_fieldOfPlants;
                    commandHarvest.m_position = &m_player->m_position;
                    commandHarvest.execute();
                }
            }
            break;
            case ObjectType::Fruit:
            {
                CommandHarvestCrop commandHarvest;
                commandHarvest.m_inventoryToAddFruitTo = m_player;
                commandHarvest.m_fieldOfPlants         = &m_fieldOfPlants;
                commandHarvest.m_position              = &m_player->m_position;
                commandHarvest.execute();
            }
            break;
            default:
            {
                CommandHarvestCrop commandHarvest;
                commandHarvest.m_inventoryToAddFruitTo = m_player;
                commandHarvest.m_fieldOfPlants         = &m_fieldOfPlants;
                commandHarvest.m_position              = &m_player->m_position;
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
            CommandAddObjectToInventory command;
            command.m_inventory   = m_player;
            command.m_objectToAdd = {ObjectType::Seed};
        }
    }

    void player_sellFruits()
    {
        auto fruits = m_player->m_slots.find(ObjectType::Fruit);
        if (fruits != m_player->m_slots.end())
        {
            for (auto fruit : fruits->second.m_objects)
            {
                AgentPlant* fruitAgent = static_cast<AgentPlant*>(fruit.m_data);
                m_player->m_money += fruitAgent->m_age * fruitAgent->m_health;
                // TODO delete agent from agentList
                // m_agents.erase(fruitAgent);
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

        for (Int i = 0; i < FIELD_SIZE; ++i)
        {
            for (Int j = 0; j < FIELD_SIZE; ++j)
            {
                AgentNutriment* nutriment = new AgentNutriment();
                m_agentManager.add_agent(nutriment);
                nutriment->m_position.x       = i;
                nutriment->m_position.y       = j;
                nutriment->m_nutrientQuantity = s_fieldMaxNutiments;
                CommandPlaceOnField commandPlaceOnField;
                commandPlaceOnField.m_field = &m_fieldOfNutriments;
                commandPlaceOnField.m_positionableToPlace = nutriment;
                commandPlaceOnField.execute();
            }
        }

        // Initialize Player
        m_player = new AgentCharacter();
        m_agentManager.add_agent(m_player);
        m_player->m_position.x = 0;
        m_player->m_position.y = 0;
        m_player->m_money      = 0.0f;

        CommandAddObjectToInventory command;
        command.m_inventory   = m_player;
        command.m_objectToAdd = {ObjectType::Seed};
        for (Int i = 0; i < 10; ++i) { command.execute(); }
        m_player->m_selectedSlot = m_player->m_slots.end();

        // Initialize Machine
        m_machine = new AgentMachine();
        m_agentManager.add_agent(m_machine);

        command.m_inventory = m_machine;
        for (Int i = 0; i < 100; ++i) { command.execute(); }
        m_machine->m_orientation                = IOrientable::Right;
        CommandPlantCrop* commandPlant          = new CommandPlantCrop();
        commandPlant->m_agentManager            = &m_agentManager;
        commandPlant->m_fieldOfNutriments       = &m_fieldOfNutriments;
        commandPlant->m_fieldOfPlants           = &m_fieldOfPlants;
        commandPlant->m_inventoryToTakeSeedFrom = m_machine;
        commandPlant->m_position                = &m_machine->m_position;
        m_machine->m_instructions.push_back(commandPlant);
        CommandMoveForward* commandMove   = new CommandMoveForward();
        commandMove->m_orientable         = m_machine;
        commandMove->m_positionableToMove = m_machine;
        m_machine->m_instructions.push_back(commandMove);
    }

    virtual void destroy() override
    {
        m_agentManager.clear();

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

        m_agentManager.update_agents(a_deltaTime);

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
            display_heroInventory(m_player);
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
            m_agentManager.display_agents();
        }
        ImGui::End();

        ImGui::Render();

        render();
        return true;
    }

    AgentCharacter* m_player;
    AgentMachine*   m_machine;
    Field           m_fieldOfPlants;
    Field           m_fieldOfNutriments;
    AgentManager    m_agentManager;

    m::input::InputManager m_inputManager;
    m::windows::IWindow*   m_mainWindow;

    const m::logging::ChannelID m_SFLOG_ID = mLOG_GET_ID();
};

M_EXECUTE_WINDOWED_APP(StardewFactoryApp)
