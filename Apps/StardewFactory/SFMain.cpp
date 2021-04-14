#include <MesumGraphics/DearImgui/imgui.h>

#include <MesumCore/Kernel/Kernel.hpp>
#include <MesumCore/Kernel/Math.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/WindowedApp.hpp>

using namespace m;

#define FIELD_SIZE 10

#define INVENTORY_SIZE 5

static const Float s_matureAge = 3.0f;
static const Float s_seedPrice = 150.0f;

struct Agent
{
    m::math::IVec2 m_position;
    m::Float       m_consumption = 0;
    m::Float       m_age         = 0.0f;
    m::Float       m_health      = 100.0f;
};

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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void add_objectToInventory(Inventory& a_inventory, Object a_object)
{
    a_inventory.m_slots[a_object.m_type].m_objects.push_back(a_object);
}

struct Character
{
    math::IVec2 m_position;
    Float       m_money;
    Inventory   m_inventory;
};

struct Field
{
    Agent* m_agents[FIELD_SIZE][FIELD_SIZE];
    Float  m_nutrients[FIELD_SIZE][FIELD_SIZE];
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void initialize_field(Field& a_field)
{
    for (Int i = 0; i < FIELD_SIZE; ++i)
    {
        for (Int j = 0; j < FIELD_SIZE; ++j)
        {
            a_field.m_nutrients[i][j] = 10.0f;
            a_field.m_agents[i][j]    = nullptr;
        }
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void initialize_hero(Character& a_hero)
{
    a_hero.m_position.x = 0;
    a_hero.m_position.y = 0;
    a_hero.m_money      = 0.0f;

    for (Int i = 0; i < 10; ++i)
    {
        add_objectToInventory(a_hero.m_inventory, {ObjectType::Seed, nullptr});
    }
    a_hero.m_inventory.m_selectedSlot = a_hero.m_inventory.m_slots.end();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void place_agent(Field& a_field, Agent* a_agent, Int a_x, Int a_y)
{
    a_agent->m_position.x  = a_x;
    a_agent->m_position.y  = a_y;
    a_agent->m_consumption = 1.0f;

    mAssert(a_field.m_agents[a_x][a_y] == nullptr);
    a_field.m_agents[a_x][a_y] = a_agent;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Bool try_plant(Field& a_field, std::set<Agent*>& a_agents, Character& a_hero)
{
    Agent* currentAgent =
        a_field.m_agents[a_hero.m_position.x][a_hero.m_position.y];
    if (currentAgent == nullptr)
    {
        auto seedSlot = a_hero.m_inventory.m_slots.find(ObjectType::Seed);

        if (seedSlot != a_hero.m_inventory.m_slots.end() &&
            seedSlot->second.m_objects.size() > 0)
        {
            Agent* newAgent = new Agent();
            a_agents.insert(newAgent);
            place_agent(a_field, newAgent, a_hero.m_position.x,
                        a_hero.m_position.y);
            seedSlot->second.m_objects.erase(
                --seedSlot->second.m_objects.end());
            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Bool try_harvest(Field& a_field, std::set<Agent*>& a_agents, Character& a_hero)
{
    Agent* currentAgent =
        a_field.m_agents[a_hero.m_position.x][a_hero.m_position.y];
    if (currentAgent != nullptr)
    {
        a_agents.erase(currentAgent);

        add_objectToInventory(a_hero.m_inventory,
                              {ObjectType::Fruit, currentAgent});

        a_field.m_agents[a_hero.m_position.x][a_hero.m_position.y] = nullptr;
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void update_agents(std::set<Agent*>& a_agentsToUpdate, Field& a_field,
                   Double a_deltaTime)
{
    for (auto& agent : a_agentsToUpdate)
    {
        m::Float& cell =
            a_field.m_nutrients[agent->m_position.x][agent->m_position.y];
        if (cell - agent->m_consumption >= 0)
        {
            cell -= agent->m_consumption * a_deltaTime;
            if (agent->m_age < s_matureAge)
            {
                agent->m_age += a_deltaTime;
            }
        }
        else
        {
            agent->m_health -= 10.0f * a_deltaTime;
        }
    }
}

static ImVec4 colField = {0.284, 0.159, 0.0f, 1.0f};

const static Float parcelSize    = 40;
const static Float parcelPadding = 3;

const static Float agentSizeSmall = 2;
const static Float agentSizeBig   = 10;

const static Float heroSize = 4;

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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void display_agents(std::set<Agent*> const& a_agents)
{
    const ImVec2 p         = ImGui::GetCursorScreenPos();
    Float        cx        = p.x + 5.0f;
    Float        cy        = p.y + 5.0f;
    ImDrawList*  draw_list = ImGui::GetWindowDrawList();

    for (auto agent : a_agents)
    {
        Float agentSize = math::lerp(agentSizeSmall, agentSizeBig,
                                     agent->m_age / s_matureAge);

        Float innerCellPadding = parcelSize / 2.0f - agentSize / 2.0f;

        Float x = cx + (parcelSize + parcelPadding) * agent->m_position.x +
                  innerCellPadding;
        Float y = cy + (parcelSize + parcelPadding) * agent->m_position.y +
                  innerCellPadding;

        Float death = 1.0f - agent->m_health / 100.0f;

        ImVec4 colf;
        if (agent->m_age < s_matureAge)
        {
            colf = ImVec4(death, 0.0f, 1.0f - death, 1.0f);
        }
        else
        {
            colf = ImVec4(death, 1.0f - death, 0.0f, 1.0f);
        }
        const ImU32 col = ImColor(colf);

        draw_list->AddRectFilled(ImVec2(x, y),
                                 ImVec2(x + agentSize, y + agentSize), col);
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void display_character(Character const& a_hero)
{
    const ImVec2 p         = ImGui::GetCursorScreenPos();
    Float        cx        = p.x + 5.0f;
    Float        cy        = p.y + 5.0f;
    ImDrawList*  draw_list = ImGui::GetWindowDrawList();

    static Float innerCellPadding = 3;

    Float x = cx + (parcelSize + parcelPadding) * a_hero.m_position.x +
              innerCellPadding;
    Float y = cy + (parcelSize + parcelPadding) * a_hero.m_position.y +
              innerCellPadding;

    ImVec4      colf = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    const ImU32 col  = ImColor(colf);

    draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + heroSize, y + heroSize),
                             col);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void display_heroInventory(Character& a_hero)
{
    for (auto slot = a_hero.m_inventory.m_slots.begin();
         slot != a_hero.m_inventory.m_slots.end(); ++slot)
    {
        std::stringstream slotName;
        slotName << ObjectTypeNames[slot->first] << " : "
                 << slot->second.m_objects.size();
        if (slot == a_hero.m_inventory.m_selectedSlot)
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
                a_hero.m_inventory.m_selectedSlot = slot;
            }
        }
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class StardewFactoryApp : public m::crossPlatform::IWindowedApplication
{
    void move_up()
    {
        if (m_hero.m_position.y > 0)
            m_hero.m_position.y--;
    }
    void move_down()
    {
        if (m_hero.m_position.y < FIELD_SIZE - 1)
            m_hero.m_position.y++;
    }
    void move_left()
    {
        if (m_hero.m_position.x > 0)
            m_hero.m_position.x--;
    }
    void move_right()
    {
        if (m_hero.m_position.x < FIELD_SIZE - 1)
            m_hero.m_position.x++;
    }

    void player_action()
    {
        if (m_hero.m_inventory.m_selectedSlot !=
            m_hero.m_inventory.m_slots.end())
        {
            switch (m_hero.m_inventory.m_selectedSlot->first)
            {
                case ObjectType::Seed:
                {
                    if (!try_plant(m_field, m_agents, m_hero))
                    {
                        try_harvest(m_field, m_agents, m_hero);
                    }
                }
                break;
                case ObjectType::Fruit:
                {
                    try_harvest(m_field, m_agents, m_hero);
                }
                break;
                default:
                {
                    try_harvest(m_field, m_agents, m_hero);
                }
                break;
            }
        }
    }

    void player_buySeed()
    {
        // m_seeds++;
        if (m_hero.m_money >= s_seedPrice)
        {
            m_hero.m_money -= s_seedPrice;
            add_objectToInventory(m_hero.m_inventory, {ObjectType::Seed});
        }
    }

    void player_sellFruits()
    {
        auto fruits = m_hero.m_inventory.m_slots.find(ObjectType::Fruit);
        if (fruits != m_hero.m_inventory.m_slots.end())
        {
            for (auto fruit : fruits->second.m_objects)
            {
                Agent* fruitAgent = static_cast<Agent*>(fruit.m_data);
                m_hero.m_money += fruitAgent->m_age * fruitAgent->m_health;
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

        initialize_field(m_field);
        initialize_hero(m_hero);
    }

    virtual void destroy() override
    {
        for (auto agent : m_agents) { delete agent; }
        m_agents.clear();

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

        update_agents(m_agents, m_field, a_deltaTime);

        start_dearImGuiNewFrame();

        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        ImGui::Begin("Engine");
        {
            ImGui::ColorEdit4("Color", &colField.x, ImGuiColorEditFlags_Float);
            ImGui::Text("frame time : %f", a_deltaTime);
            ImGui::Text("total time : %f", s_time);
        }
        ImGui::End();

        ImGui::Begin("Inventory");
        {
            display_heroInventory(m_hero);
        }
        ImGui::End();

        ImGui::Begin("Stats");
        {
            ImGui::Text("Money : %f", m_hero.m_money);

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
            display_agents(m_agents);
            display_character(m_hero);
        }
        ImGui::End();

        ImGui::Render();

        render();
        return true;
    }

    Field            m_field;
    Character        m_hero;
    std::set<Agent*> m_agents;

    m::input::InputManager m_inputManager;
    m::windows::IWindow*   m_mainWindow;

    const m::logging::ChannelID m_SFLOG_ID = mLOG_GET_ID();
};

M_EXECUTE_WINDOWED_APP(StardewFactoryApp)
