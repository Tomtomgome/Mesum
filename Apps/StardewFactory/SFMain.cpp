#include <MesumGraphics/DearImgui/imgui.h>

#include <MesumCore/Kernel/Kernel.hpp>
#include <MesumCore/Kernel/Math.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/WindowedApp.hpp>

#include <Agents.hpp>
#include <Rendering.hpp>

using namespace m;



math::RandomGenerator g_numberGenerator;


// enum ObjectType
// {
//     Hoe = 0,
//     WateringCan,
//     Seed,
//     Fruit,
// };
// 
// static std::string ObjectTypeNames[] = {"Hoe", "Watering Can", "Seeds", "Fruits"};
// 
// struct Object
// {
//     ObjectType m_type;
//     void*      m_data;
// };
// 
// struct Slot
// {
//     std::vector<Object> m_objects;
// };
// 
// struct IInventory
// {
//     std::map<ObjectType, Slot>::iterator m_selectedSlot;
//     std::map<ObjectType, Slot>           m_slots;
// };

// struct CommandAddObjectToInventory : ICommand
// {
//     Bool execute()
//     {
//         m_inventory->m_slots[m_objectToAdd.m_type].m_objects.push_back(
//             m_objectToAdd);
//         return true;
//     }
// 
//     IInventory* m_inventory;
//     Object      m_objectToAdd;
// };

//*****************************************************************************
// Crops
//*****************************************************************************
// struct AgentNutriment : public IPositionable, public IPermanent
// {
//     void update(Double a_deltaTime)
//     {
//         m_nutrientQuantity +=
//             std::min(s_fieldRegenerationRate * static_cast<Float>(a_deltaTime),
//                      s_fieldMaxNutiments);
//     }
// 
//     Float m_nutrientQuantity = s_fieldMaxNutiments;
// };


// struct AgentPlant : public IPositionable, public IVolatile
// {
//     void update(Double a_deltaTime)
//     {
//         if (m_isHarvested)
//         {
//             m_health = std::max(
//                 m_health - s_plantDeathRateWhenHarvested * a_deltaTime, 0.0);
//             return;
//         }
// 
//         Field::Cell& cellContent =
//             m_fieldOfNutriment->m_cells[m_position.x][m_position.y];
//         AgentNutriment* nutriment =
//             static_cast<AgentNutriment*>(*cellContent.begin());
// 
//         if (nutriment->m_nutrientQuantity - m_consumption >= 0)
//         {
//             nutriment->m_nutrientQuantity -= m_consumption * a_deltaTime;
//             if (m_age < s_matureAge)
//             {
//                 m_age += a_deltaTime;
//             }
//         }
//         else
//         {
//             m_health = std::max(
//                 m_health - s_plantDeathRateWhenGrounded * a_deltaTime, 0.0);
//         }
//     }
// 
//     void harvest() { m_isHarvested = true; }
// 
//     Field* m_fieldOfNutriment = nullptr;
//     Bool   m_isHarvested      = false;
//     Float  m_consumption      = s_plantBaseConsumptionRate;
//     Float  m_age              = 0.0f;
//     Float  m_health           = s_plantBaseHealth;
// };

//*****************************************************************************
// Crop related commands
//*****************************************************************************
// struct CommandPlantCrop : public ICommand
// {
//     virtual Bool execute() override
//     {
//         // Look if there is already a plant
//         Field::Cell& cellContent =
//             m_fieldOfPlants->m_cells[m_positionToPlant->m_position.x]
//                                     [m_positionToPlant->m_position.y];
// 
//         // Look if there is already a plant
//         if (cellContent.begin() != cellContent.end())
//         {
//             return false;
//         }
// 
//         auto seedSlot =
//             m_inventoryToTakeSeedFrom->m_slots.find(ObjectType::Seed);
// 
//         if (seedSlot == m_inventoryToTakeSeedFrom->m_slots.end() ||
//             seedSlot->second.m_objects.size() == 0)
//         {
//             return false;
//         }
// 
//         // create a plant agent
//         AgentPlant* newPlant = new AgentPlant();
//         m_agentManager->add_agent(newPlant);
// 
//         newPlant->m_fieldOfNutriment = m_fieldOfNutriments;
//         newPlant->m_position         = m_positionToPlant->m_position;
// 
//         CommandPlaceOnField placeOnField;
//         placeOnField.m_field               = m_fieldOfPlants;
//         placeOnField.m_positionableToPlace = newPlant;
//         placeOnField.execute();
// 
//         seedSlot->second.m_objects.erase(--seedSlot->second.m_objects.end());
//         return true;
//     }
// 
//     AgentManager*  m_agentManager;
//     Field*         m_fieldOfPlants;
//     Field*         m_fieldOfNutriments;
//     IInventory*    m_inventoryToTakeSeedFrom;
//     IPositionable* m_positionToPlant;
// };
// 
// struct CommandHarvestCrop : public ICommand
// {
//     virtual Bool execute() override
//     {
//         // Look if there is allready a plant
//         Field::Cell& cellContent =
//             m_fieldOfPlants->m_cells[m_positionToHarvest->m_position.x]
//                                     [m_positionToHarvest->m_position.y];
// 
//         // Look if there is no plant
//         if (cellContent.begin() == cellContent.end())
//         {
//             return false;
//         }
// 
//         AgentPlant* plantToHarvest =
//             static_cast<AgentPlant*>(*cellContent.begin());
//         // Harvest the plant
//         plantToHarvest->harvest();
// 
//         CommandAddObjectToInventory commandAddObject;
//         commandAddObject.m_inventory   = m_inventoryToAddFruitTo;
//         commandAddObject.m_objectToAdd = {ObjectType::Fruit, plantToHarvest};
//         commandAddObject.execute();
// 
//         cellContent.erase(plantToHarvest);
// 
//         return true;
//     }
// 
//     Field*         m_fieldOfPlants;
//     IInventory*    m_inventoryToAddFruitTo;
//     IPositionable* m_positionToHarvest;
// };

// *****************************************************************************
// Machines
// *****************************************************************************
// struct AgentMachine : public IPositionable,
//                       public IInventory,
//                       public IOrientable,
//                       public IPermanent
// {
//     virtual ~AgentMachine()
//     {
//         for (auto command : m_instructions) { delete command; }
//         m_instructions.clear();
//     }
// 
//     void update(Double a_deltaTime)
//     {
//         if (m_instructions.size() == 0)
//         {
//             return;
//         }
// 
//         m_timeBeforeNextStep -= a_deltaTime;
//         if (m_timeBeforeNextStep > 0)
//         {
//             return;
//         }
// 
//         m_timeBeforeNextStep = s_machineRefreshTime;
// 
//         m_instructions[m_instructionToExecute]->execute();
//         m_instructionToExecute =
//             (m_instructionToExecute + 1U) % m_instructions.size();
//     }
// 
//     Float                  m_timeBeforeNextStep   = s_machineRefreshTime;
//     U64                    m_instructionToExecute = 0;
//     std::vector<ICommand*> m_instructions;
// };

//*****************************************************************************
// Machines
//*****************************************************************************
struct LogicalWorld
{
    Field m_fieldOfPlants;
    Field m_fieldOfSoil;
};



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void display_heroInventory(IInventory* a_inventory)
{
    for (auto slot = a_inventory->m_slots.begin();
         slot != a_inventory->m_slots.end(); ++slot)
    {
        std::stringstream slotName;
        slotName << slot->second.m_items.back()->get_name() << " : "
                 << slot->second.m_items.size();
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
            return;

        m_player->m_selectedSlot->second.m_items.back()->use();
//         if (m_player->m_selectedSlot == m_player->m_slots.end())
//         {
//             CommandHarvestCrop commandHarvest;
//             commandHarvest.m_inventoryToAddFruitTo = m_player;
//             commandHarvest.m_fieldOfPlants         = &m_fieldOfPlants;
//             commandHarvest.m_positionToHarvest     = m_player;
//             commandHarvest.execute();
//             return;
//         }
// 
//         switch (m_player->m_selectedSlot->first)
//         {
//             case ObjectType::Seed:
//             {
//                 CommandPlantCrop commandPlant;
//                 commandPlant.m_agentManager            = &m_agentManager;
//                 commandPlant.m_fieldOfPlants           = &m_fieldOfPlants;
//                 commandPlant.m_fieldOfNutriments       = &m_fieldOfSoil;
//                 commandPlant.m_inventoryToTakeSeedFrom = m_player;
//                 commandPlant.m_positionToPlant         = m_player;
//                 if (!commandPlant.execute())
//                 {
//                     CommandHarvestCrop commandHarvest;
//                     commandHarvest.m_inventoryToAddFruitTo = m_player;
//                     commandHarvest.m_fieldOfPlants         = &m_fieldOfPlants;
//                     commandHarvest.m_positionToHarvest     = m_player;
//                     commandHarvest.execute();
//                 }
//             }
//             break;
//             case ObjectType::Fruit:
//             {
//                 CommandHarvestCrop commandHarvest;
//                 commandHarvest.m_inventoryToAddFruitTo = m_player;
//                 commandHarvest.m_fieldOfPlants         = &m_fieldOfPlants;
//                 commandHarvest.m_positionToHarvest     = m_player;
//                 commandHarvest.execute();
//             }
//             break;
//             default:
//             {
//                 CommandHarvestCrop commandHarvest;
//                 commandHarvest.m_inventoryToAddFruitTo = m_player;
//                 commandHarvest.m_fieldOfPlants         = &m_fieldOfPlants;
//                 commandHarvest.m_positionToHarvest     = m_player;
//                 commandHarvest.execute();
//             }
//             break;
//         }
    }

    void player_buySeed()
    {
//         if (m_player->m_money >= s_seedPrice)
//         {
//             m_player->m_money -= s_seedPrice;
//             CommandAddObjectToInventory command;
//             command.m_inventory   = m_player;
//             command.m_objectToAdd = {ObjectType::Seed};
//             command.execute();
//         }
    }

    void player_sellFruits()
    {
//         auto fruits = m_player->m_slots.find(ObjectType::Fruit);
//         if (fruits != m_player->m_slots.end())
//         {
//             for (auto fruit : fruits->second.m_objects)
//             {
//                 AgentPlant* fruitAgent = static_cast<AgentPlant*>(fruit.m_data);
//                 m_player->m_money += fruitAgent->m_age * fruitAgent->m_health;
//                 fruitAgent->ask_deletion();
//                 delete fruitAgent;
//             }
//             fruits->second.m_objects.clear();
//         }
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

        g_numberGenerator.init(0);

        for (Int i = 0; i < FIELD_SIZE; ++i)
        {
            for (Int j = 0; j < FIELD_SIZE; ++j)
            {
//                 AgentNutriment* nutriment = new AgentNutriment();
//                 m_agentManager.add_agent(nutriment);
//                 nutriment->m_position.x       = i;
//                 nutriment->m_position.y       = j;
//                 nutriment->m_nutrientQuantity = s_fieldMaxNutiments;
//                 CommandPlaceOnField commandPlaceOnField;
//                 commandPlaceOnField.m_field = &m_fieldOfSoil;
//                 commandPlaceOnField.m_positionableToPlace = nutriment;
//                 commandPlaceOnField.execute();

                AgentSoil* soil = new AgentSoil();
                m_agentManager.add_agent(soil);
                soil->m_position.x       = i;
                soil->m_position.y       = j;
                soil->m_nutrientQuantity = s_fieldMaxNutiments;
                soil->m_isOccupied       = false;
                soil->m_type             = AgentSoil::Grass;
                soil->m_fieldOfSoil      = &m_logicalWorld.m_fieldOfSoil;
                CommandPlaceOnField commandPlaceOnField;
                commandPlaceOnField.m_field = &m_logicalWorld.m_fieldOfSoil;
                commandPlaceOnField.m_positionableToPlace = soil;
                commandPlaceOnField.execute();
            }
        }

//         static_cast<AgentSoil*>(*(m_fieldOfSoil.m_cells[0][0].begin()))->m_type =
//             AgentSoil::Grass;

        // Initialize Player
        m_player = new AgentCharacter();
        m_agentManager.add_agent(m_player);
        m_player->m_position.x = 0;
        m_player->m_position.y = 0;
        m_player->m_money      = 1000.0f;
        m_player->m_selectedSlot = m_player->m_slots.end();
        
        ItemHoe* hoe = new ItemHoe();
        hoe->m_fieldOfSoil  = &m_logicalWorld.m_fieldOfSoil;
        hoe->m_positionable = m_player;
        CommandAddObjectToInventory commandInventory;
        commandInventory.m_inventory = m_player;
        commandInventory.m_itemToAdd = hoe;
        commandInventory.execute();
        ItemWateringCan* wateringCan = new ItemWateringCan();
        wateringCan->m_fieldOfSoil   = &m_logicalWorld.m_fieldOfSoil;
        wateringCan->m_positionable  = m_player;
        commandInventory.m_itemToAdd = wateringCan;
        commandInventory.execute();

        // Initialize Machine
//         m_machine = new AgentMachine();
//         m_agentManager.add_agent(m_machine);
// 
//         command.m_inventory = m_machine;
//         for (Int i = 0; i < 100; ++i) { command.execute(); }
//         m_machine->m_orientation                = IOrientable::Right;
//         CommandPlantCrop* commandPlant          = new CommandPlantCrop();
//         commandPlant->m_agentManager            = &m_agentManager;
//         commandPlant->m_fieldOfNutriments       = &m_fieldOfNutriments;
//         commandPlant->m_fieldOfPlants           = &m_fieldOfPlants;
//         commandPlant->m_inventoryToTakeSeedFrom = m_machine;
//         commandPlant->m_positionToPlant         = m_machine;
//         m_machine->m_instructions.push_back(commandPlant);
//         CommandMoveForward* commandMove   = new CommandMoveForward();
//         commandMove->m_orientable         = m_machine;
//         commandMove->m_positionableToMove = m_machine;
//         m_machine->m_instructions.push_back(commandMove);
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
            //ImGui::ColorEdit4("Color", &colField.x, ImGuiColorEditFlags_Float);
            ImGui::Text("frame time : %f", a_deltaTime);
            ImGui::Text("frame FPS : %f", 1.0 / a_deltaTime);
            ImGui::Text("total time : %f", s_time);
            ImGui::DragFloat("Machine refresh time", &s_machineRefreshTime,
                             0.01f, 0.0f);
            if (ImGui::Button("Turn clock"))
            {
                //m_machine->rotation_clockwise();
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
            m_agentManager.render_agents();
            g_world.display();
        }
        ImGui::End();

        ImGui::Render();

        render();
        return true;
    }

    AgentCharacter* m_player;
    //AgentMachine*   m_machine;
    LogicalWorld    m_logicalWorld;
    AgentManager    m_agentManager;

    m::input::InputManager m_inputManager;
    m::windows::IWindow*   m_mainWindow;

    const m::logging::ChannelID m_SFLOG_ID = mLOG_GET_ID();
};

M_EXECUTE_WINDOWED_APP(StardewFactoryApp)
