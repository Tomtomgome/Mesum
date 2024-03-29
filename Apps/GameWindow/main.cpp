#include <Kernel/File.hpp>
#include <Kernel/Math.hpp>
#include <Kernel/MatHelpers.hpp>
#include <Kernel/Memory.hpp>
#include <MesumCore/Kernel/Image.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/DX12Renderer/DX12Context.hpp>
#include <MesumGraphics/DearImgui/MesumDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTask2DRender.hpp>
#include <MesumGraphics/RenderTasks/RenderTaskDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTasksBasicSwapchain.hpp>
#include <MesumGraphics/VulkanRenderer/VulkanContext.hpp>
#include <MesumGraphics/RendererUtils.hpp>

#include "Scene.hpp"
#include "GameActionDef.hpp"

#include <filesystem>

#define ENABLE_EDITOR

using namespace m;

static const mInt screenWidth  = 400;
static const mInt screenHeight = 300;
static mInt       windowWidth  = 0;
static mInt       windowHeight = 0;

math::mXoRandomNumberGenerator g_randomGenerator(0);

struct GenerationData
{
    m::math::mVec4 color;
    m::math::mVec3 offset;
    m::mFloat      angle;
    m::mFloat      size;
};

m::math::mVec3 convert_radToCoordinates(m::mFloat const a_angle,
                                        m::mFloat const a_radius)
{
    return {a_radius * cosf(a_angle), a_radius * sinf(a_angle), 0.5f};
}

void generate_squareIntoMeshBuffer(
    render::DataMeshBuffer<render::BasicVertex, mU16>* a_meshBuffer,
    DrawableData const&                                a_generationInfo)
{
    // Indexes
    mUInt index = a_meshBuffer->m_vertices.size();
    a_meshBuffer->m_indices.push_back(index);
    a_meshBuffer->m_indices.push_back(index + 1);
    a_meshBuffer->m_indices.push_back(index + 2);
    a_meshBuffer->m_indices.push_back(index + 3);
    a_meshBuffer->m_indices.push_back(0xFFFF);

    // Prep data
    static m::mFloat pi   = 3.14159265;
    static m::mFloat step = pi / 2.0f;

    m::math::mVec3 offset       = a_generationInfo.offset;
    m::mFloat      radius       = a_generationInfo.size / std::sqrt(2.0f);
    m::mFloat      initialAngle = (pi / 4.0f) + a_generationInfo.angle;

    // Vertices
    render::BasicVertex vertex;
    vertex.color = a_generationInfo.color;
    vertex.position =
        offset + convert_radToCoordinates(initialAngle + 2 * step, radius);
    vertex.uv = {0.0f, 1.0f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position =
        offset + convert_radToCoordinates(initialAngle + step, radius);
    vertex.uv = {0.0f, 0.0f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position =
        offset + convert_radToCoordinates(initialAngle + 3 * step, radius);
    vertex.uv = {1.0f, 1.0f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position = offset + convert_radToCoordinates(initialAngle, radius);
    vertex.uv       = {1.0f, 0.0f};
    a_meshBuffer->m_vertices.push_back(vertex);
}

void generate_rectangleIntoMeshBuffer(
    render::DataMeshBuffer<render::BasicVertex, mU16>* a_meshBuffer,
    math::mVec3 const& a_offset, math::mVec2 const& a_size,
    math::mVec4 const& a_color)
{
    // Indexes
    mUInt index = a_meshBuffer->m_vertices.size();
    a_meshBuffer->m_indices.push_back(index);
    a_meshBuffer->m_indices.push_back(index + 1);
    a_meshBuffer->m_indices.push_back(index + 2);
    a_meshBuffer->m_indices.push_back(index + 3);
    a_meshBuffer->m_indices.push_back(0xFFFF);

    // Vertices
    render::BasicVertex vertex;
    vertex.color    = a_color;
    vertex.position = a_offset + math::mVec3{0.0f, -a_size.y, 0.0f};
    vertex.uv       = {0.0f, 1.0f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position = a_offset;
    vertex.uv       = {0.0f, 0.0f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position = a_offset + math::mVec3{a_size.x, -a_size.y, 0.0f};
    vertex.uv       = {1.0f, 1.0f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position = a_offset + math::mVec3{a_size.x, 0.0f, 0.0f};
    vertex.uv       = {1.0f, 0.0f};
    a_meshBuffer->m_vertices.push_back(vertex);
}

class mTargetController
{
   public:
    void activate_moveMode(const math::mIVec2& a_position)
    {
        m_middleButtonPressed = true;
    }

    void deactivate_moveMode(const math::mIVec2& a_position)
    {
        m_middleButtonPressed = false;
    }

    void move_target(const math::mIVec2& a_position)
    {
        if (m_middleButtonPressed)
        {
            m_targetPoint.x += a_position.x / m_zoomLevel;
            m_targetPoint.y -= a_position.y / m_zoomLevel;
            update_worldToViewMatrix();
        }
    }

    void set_target(const math::mIVec2& a_position)
    {
        m_targetPoint.x = a_position.x - windowWidth / 2;
        m_targetPoint.y = a_position.y - 1080 + windowHeight / 2;
        update_worldToViewMatrix();
    }

    void update_zoomLevel(mDouble a_update)
    {
        m_zoomPower -= a_update;
        m_zoomLevel = 1 * std::pow(m_speed, m_zoomPower);
        update_worldToViewMatrix();
    }

    void update_worldToViewMatrix()
    {
        m_worldToView =
            math::generate_scaleMatrix(m_zoomLevel, m_zoomLevel, 1.0) *
            math::generate_translationMatrix(m_targetPoint.x, m_targetPoint.y,
                                             0.0f);

        m_viewToWorld =
            math::generate_translationMatrix(-m_targetPoint.x, -m_targetPoint.y,
                                             0.0f) *
            math::generate_scaleMatrix(1 / m_zoomLevel, 1 / m_zoomLevel, 1.0);
    }

    mBool         m_middleButtonPressed = false;
    mFloat        m_speed{0.75};
    mFloat        m_zoomPower{0};
    mFloat        m_zoomLevel{1};
    math::mIVec2  m_zoomOffset{0, 0};
    math::mIVec2  m_targetPoint{0, 0};
    math::mIVec2  m_finalPoint{0, 0};
    math::mMat4x4 m_worldToView{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    math::mMat4x4 m_viewToWorld{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
};

struct PlayerHand
{
    void hold_things(const math::mIVec2& a_position)
    {
        heldPosition    = a_position;
        requestGripping = true;
    }

    void move_hand(const math::mIVec2& a_position)
    {
        requestMove = true;
        displacement += {a_position.x, -a_position.y};
    }

    void release_things(const math::mIVec2& a_position)
    {
        requestRelease = true;
    }

    math::mIVec2 displacement{};
    math::mIVec2 heldPosition{};
    mBool        requestGripping = false;
    mBool        requestMove     = false;
    mBool        requestRelease  = false;

    std::vector<Entity> hand;
};

class RendererTestApp : public m::crossPlatform::IWindowedApplication
{
    static const m::mUInt s_nbBackbuffer = 3;

    void init_game()
    {
        m_pGameApi = new m::vulkan::mApi();
        m_pGameApi->init();
        auto& gameApi = m::unref_safe(m_pGameApi);

        auto& gameSynchTool = gameApi.create_synchTool();
        m_pGameSynchTool    = &gameSynchTool;
        m::render::mISynchTool::Desc desc{s_nbBackbuffer};
        gameSynchTool.init(desc);

        auto& gameSwapchain = gameApi.create_swapchain();
        m_pGameSwapchain    = &gameSwapchain;

        // Setup game window
        m_windowGame = static_cast<win32::IWindowImpl*>(
            add_newWindow("Game", screenWidth, screenHeight, true));
        m_windowGame->link_inputManager(&m_inputManagerGame);

        render::init_swapchainWithWindow(gameApi, m_tastsetExecutor,
                                         gameSwapchain, gameSynchTool,
                                         *m_windowGame, s_nbBackbuffer);

        auto& gameTaskset = gameApi.create_renderTaskset();

        m::render::mTaskDataSwapchainWaitForRT taskData_swapchainWaitForRT{};
        taskData_swapchainWaitForRT.pSwapchain = m_pGameSwapchain;
        taskData_swapchainWaitForRT.pSynchTool = m_pGameSynchTool;
        auto& imageAcquireTask =
            static_cast<m::render::mTaskSwapchainWaitForRT&>(
                taskData_swapchainWaitForRT.add_toTaskSet(gameTaskset));

        render::TaskData2dRender taskData_2dRender;
        taskData_2dRender.m_pMeshBuffer = &m_meshBuffer;
        taskData_2dRender.m_pRanges     = &m_ranges;
        taskData_2dRender.nbFrames      = s_nbBackbuffer;
        taskData_2dRender.pOutputRT     = imageAcquireTask.pOutputRT;
        taskData_2dRender.m_pMatrix     = &m_matrixGame;
        auto& taskRenderGame            = static_cast<render::Task2dRender&>(
            taskData_2dRender.add_toTaskSet(gameTaskset));
        m_pTaskRenderGame = &taskRenderGame;

        taskRenderGame.add_texture(m_imageRequested[0]);
        taskRenderGame.add_texture(m_imageRequested[1]);

        m::render::mTaskDataSwapchainPresent taskData_swapchainPresent{};
        taskData_swapchainPresent.pSwapchain = m_pGameSwapchain;
        taskData_swapchainPresent.pSynchTool = m_pGameSynchTool;
        taskData_swapchainPresent.add_toTaskSet(gameTaskset);

        m_tastsetExecutor.confy_permanentTaskset(gameApi, gameTaskset);
        m_windowGame->attach_toDestroy(m::mCallback<void>(
            [this, &gameApi, &gameTaskset]() {
                m_tastsetExecutor.remove_permanentTaskset(gameApi, gameTaskset);
            }));

        ((win32::IWindowImpl*)(m_windowGame))
            ->attach_toSpecialUpdate(mCallback(this, &RendererTestApp::render));
        ::GetWindowRect(m_windowGame->get_hwnd(), &m_initialClientRect);

        // Special game window positionning
        MONITORINFO monitorInfo = {};
        monitorInfo.cbSize      = sizeof(MONITORINFO);

        ::GetMonitorInfo(::MonitorFromWindow(m_windowGame->get_hwnd(),
                                             MONITOR_DEFAULTTONEAREST),
                         &monitorInfo);

        RECT clientRect = {};
        ::GetWindowRect(((win32::IWindowImpl*)(m_windowGame))->get_hwnd(),
                        &clientRect);
        windowWidth  = clientRect.right - clientRect.left;
        windowHeight = clientRect.bottom - clientRect.top;
        mInt xPos    = (monitorInfo.rcMonitor.right - windowWidth) / 2;
        mInt yPos    = (monitorInfo.rcMonitor.bottom - windowHeight) / 2;

        SetWindowPos(m_windowGame->get_hwnd(), NULL, xPos, yPos, windowWidth,
                     windowHeight, SWP_SHOWWINDOW | SWP_DRAWFRAME);

        m_inputManagerGame.attach_toMouseEvent(
            input::mMouseAction::mousePressed(input::mMouseButton::left),
            input::mMouseActionCallback(&m_playerHand,
                                        &PlayerHand::hold_things));
        m_inputManagerGame.attach_toMouseEvent(
            input::mMouseAction::mouseReleased(input::mMouseButton::left),
            input::mMouseActionCallback(&m_playerHand,
                                        &PlayerHand::release_things));
        m_inputManagerGame.attach_toMouseMoveEvent(
            input::mMouseActionCallback(&m_playerHand, &PlayerHand::move_hand));

        std::filesystem::path currentPath = std::filesystem::current_path();
        std::filesystem::path levelPath{currentPath / "data" / "levels" /
                                        "test.lvl"};
        m_componentManager.reset();
        m_componentManager.load_fromFile(levelPath.string());

        m_start = std::chrono::high_resolution_clock::now();
    }

    void init_editor()
    {
#ifdef ENABLE_EDITOR
        m_updateScene = false;

        m_imageRequestedEditor.emplace_back();
        m_imageRequestedEditor.back().path = "data/textures/Editor/Overlay.png";

        m_pEditorApi = new m::dx12::mApi();
        m_pEditorApi->init();
        auto& editorApi = m::unref_safe(m_pEditorApi);

        auto& editorSynchTool = editorApi.create_synchTool();
        m_pEditorSynchTool    = &editorSynchTool;
        m::render::mISynchTool::Desc desc{s_nbBackbuffer};
        editorSynchTool.init(desc);

        auto& editorSwapchain = editorApi.create_swapchain();
        m_pEditorSwapchain    = &editorSwapchain;

        // Setup editor window
        m_windowEditor = static_cast<win32::IWindowImpl*>(
            add_newWindow("Editor", 1280, 720, false));
        m_windowEditor->link_inputManager(&m_inputManagerEditor);

        m::render::init_swapchainWithWindow(editorApi, m_tastsetExecutor,
                                            editorSwapchain, editorSynchTool,
                                            *m_windowEditor, s_nbBackbuffer);

        dearImGui::init(*m_windowEditor);

        auto& editorTaskset = editorApi.create_renderTaskset();

        m::render::mTaskDataSwapchainWaitForRT taskData_swapchainWaitForRT{};
        taskData_swapchainWaitForRT.pSwapchain = m_pEditorSwapchain;
        taskData_swapchainWaitForRT.pSynchTool = m_pEditorSynchTool;
        auto& imageAcquireTask =
            static_cast<m::render::mTaskSwapchainWaitForRT&>(
                taskData_swapchainWaitForRT.add_toTaskSet(editorTaskset));

        render::TaskData2dRender taskData_2dRender;
        taskData_2dRender.m_pMeshBuffer = &m_meshBuffer;
        taskData_2dRender.m_pRanges     = &m_ranges;
        taskData_2dRender.nbFrames      = s_nbBackbuffer;
        taskData_2dRender.pOutputRT     = imageAcquireTask.pOutputRT;
        taskData_2dRender.m_pMatrix     = &m_matrixEditor;
        auto& taskRenderGameInEditor    = static_cast<render::Task2dRender&>(
            taskData_2dRender.add_toTaskSet(editorTaskset));
        m_pTaskRenderGameInEditor = &taskRenderGameInEditor;
        m_pTaskRenderGameInEditor->add_texture(m_imageRequested[0]);
        m_pTaskRenderGameInEditor->add_texture(m_imageRequested[1]);

        taskData_2dRender.m_pMeshBuffer = &m_meshBufferEditor;
        taskData_2dRender.m_pRanges     = &m_rangesEditor;
        auto& taskRenderEditor          = static_cast<render::Task2dRender&>(
            taskData_2dRender.add_toTaskSet(editorTaskset));
        taskRenderEditor.add_texture(m_imageRequestedEditor[0]);

        m::render::TaskDataDrawDearImGui taskData_drawDearImGui;
        taskData_drawDearImGui.pOutputRT = imageAcquireTask.pOutputRT;
        taskData_drawDearImGui.nbFrames  = s_nbBackbuffer;
        taskData_drawDearImGui.add_toTaskSet(editorTaskset);

        m::render::mTaskDataSwapchainPresent taskData_swapchainPresent{};
        taskData_swapchainPresent.pSwapchain = m_pEditorSwapchain;
        taskData_swapchainPresent.pSynchTool = m_pEditorSynchTool;
        taskData_swapchainPresent.add_toTaskSet(editorTaskset);

        m_tastsetExecutor.confy_permanentTaskset(editorApi, editorTaskset);
        m_windowEditor->attach_toDestroy(m::mCallback<void>(
            [this, &editorApi, &editorTaskset]() {
                editorWindowIsDead = true;
                m_tastsetExecutor.remove_permanentTaskset(editorApi,
                                                          editorTaskset);
            }));

        m_inputManagerEditor.attach_toMouseEvent(
            input::mMouseAction::mousePressed(input::mMouseButton::middle),
            input::mMouseActionCallback(&m_targetController,
                                        &mTargetController::activate_moveMode));
        m_inputManagerEditor.attach_toMouseEvent(
            input::mMouseAction::mouseReleased(input::mMouseButton::middle),
            input::mMouseActionCallback(
                &m_targetController, &mTargetController::deactivate_moveMode));
        m_inputManagerEditor.attach_toMouseMoveEvent(
            input::mMouseActionCallback(&m_targetController,
                                        &mTargetController::move_target));

        m_inputManagerEditor.attach_toMouseScrollEvent(input::mScrollCallback(
            &m_targetController, &mTargetController::update_zoomLevel));

#endif  // ENABLE_EDITOR
    }

    void init(mCmdLine const& a_cmdLine, void* a_appData) override
    {
        crossPlatform::IWindowedApplication::init(a_cmdLine, a_appData);

        m_imageRequested.emplace_back();
        m_imageRequested.back().path = "data/textures/Mir.png";
        m_imageRequested.emplace_back();
        m_imageRequested.back().path = "data/textures/Character.png";
        m_imageRequested.emplace_back();
        m_imageRequested.back().path.resize(512);  // prep for imGui

        g_animationBank.load();
        g_modelBank.load();

        m_tastsetExecutor.init();

        init_editor();
        init_game();
    }

    void destroy() override
    {
        crossPlatform::IWindowedApplication::destroy();

        g_animationBank.unload();
        g_modelBank.unload();

        m_scene.reset();
        m_componentManager.reset();

        m_tastsetExecutor.destroy();

        // call to destroy of the swapchain is managed at window termination
        m_pGameApi->destroy_swapchain(*m_pGameSwapchain);

        m_pGameSynchTool->destroy();
        m_pGameApi->destroy_synchTool(*m_pGameSynchTool);

        m_pGameApi->destroy();
        delete m_pGameApi;

#ifdef ENABLE_EDITOR
        // call to destroy of the swapchain is managed at window termination
        m_pEditorApi->destroy_swapchain(*m_pEditorSwapchain);

        m_pEditorSynchTool->destroy();
        m_pEditorApi->destroy_synchTool(*m_pEditorSynchTool);

        m_pEditorApi->destroy();
        delete m_pEditorApi;
#endif  // ENABLE_EDITOR

        dearImGui::destroy();
    }
#ifdef ENABLE_EDITOR
    void render_editorGUI()
    {
        start_dearImGuiNewFrame(*m_pEditorApi);

        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(),
                                     ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::ShowDemoWindow();

        ImGui::Begin("Engine");
        {
            ImGui::Text("Camera offset : %dx;%dy",
                        m_targetController.m_targetPoint.x,
                        m_targetController.m_targetPoint.y);
            ImGui::Text("Zoom : %f", m_targetController.m_zoomLevel);
            ImGui::Text("Default memory allocated : %d",
                        mInt(memory::g_memStats.globalAllocationSizes));
            ImGui::Checkbox("Allow Update", &m_updateScene);
            ImGui::Checkbox("Collision Debug Draw", &m_debugCollisions);

            if (ImGui::Button("Reset scene"))
            {
                m_componentManager.reset();
            }
        }
        ImGui::End();

        ImGui::Begin("Level Browser");
        {
            std::filesystem::path currentPath = std::filesystem::current_path();
            std::filesystem::path levelPath{currentPath / "data" / "levels"};
            for (const auto& entry :
                 std::filesystem::directory_iterator{levelPath})
            {
                if (entry.path().has_extension() &&
                    entry.path().extension() == ".lvl")
                {
                    if (ImGui::Button(entry.path().stem().string().c_str()))
                    {
                        m_componentManager.reset();
                        m_componentManager.load_fromFile(entry.path().string());
                    }
                }
            }
        }
        ImGui::End();

        ImGui::Begin("Level Editor", NULL, ImGuiWindowFlags_AlwaysAutoResize);
        {
            static char name[512] = "";
            ImGui::InputText("Level Name : ", name, 512);

            if (ImGui::Button("Save as"))
            {
                std::string path = "data/levels/";
                m_componentManager.save_toFile(path + name + ".lvl");
            }

            if (ImGui::CollapsingHeader("Entities"))
            {
                m_componentManager.display_gui();
            }
        }
        ImGui::End();

        ImGui::Begin("Animation Bank");
        {
            g_animationBank.display_gui();
        }
        ImGui::End();

        ImGui::Begin("Model Bank");
        {
            g_modelBank.display_gui();
        }
        ImGui::End();

        ImGui::Render();
    }
#endif  // ENABLE_EDITOR

    void update_gameScene(
        std::chrono::steady_clock::duration const& a_deltaTime)
    {
        std::vector<AnimatorEvent> animatorEvents;
        process_animatedObjects(m_componentManager.animators, animatorEvents,
                                a_deltaTime);
        process_animatorEvents(animatorEvents, m_componentManager.animators,
                               m_gameActions);
    }

    void process_playerHand(std::vector<GameAction*>& a_gameActions)
    {
        if (m_playerHand.requestRelease)
        {
            m_playerHand.requestRelease = false;
            m_playerHand.hand.clear();
        }

        if (m_playerHand.requestGripping)
        {
            m_playerHand.requestGripping = false;
            math::mIVec2& holdPosition   = m_playerHand.heldPosition;

            if (m_componentManager.entityCount > 0)
            {
                RECT clientRect = {};
                ::GetWindowRect(
                    ((win32::IWindowImpl*)(m_windowGame))->get_hwnd(),
                    &clientRect);

                math::mIVec2 targetPoint{0, 0};
                targetPoint.x = -clientRect.left - windowWidth / 2;
                targetPoint.y = clientRect.top - 1080 + windowHeight / 2;

                math::mMat4x4 translate = math::generate_translationMatrix(
                    -targetPoint.x, -targetPoint.y, 0.0f);

                math::mVec4 transPoint = {
                    mFloat(holdPosition.x - screenWidth / 2),
                    mFloat(screenHeight / 2 - holdPosition.y), 0.0f, 1.0f};

                transPoint                  = translate * transPoint;
                math::mVec2 positionInSpace = {transPoint.x, transPoint.y};

                gather_intersectedObjects(m_completeCollisionDatas,
                                          positionInSpace, m_playerHand.hand);

                for (auto& entity : m_playerHand.hand)
                {
                    GAKillEntity::InternalData internalData{};
                    internalData.entityToKill = entity;
                    a_gameActions.push_back(GAKillEntity::create(internalData));

                    if (m_componentManager.animators[entity].enabled)
                    {
                        // TODO : "Start animation"
                        m_componentManager.animators[entity].animationID  = 0;
                        m_componentManager.animators[entity].lastKeyIndex = -1;
                        m_componentManager.animators[entity].lastKeyIndex =
                            0.0f;
                    }
                }
            }
        }

        if (m_playerHand.requestMove)
        {
            m_playerHand.requestMove = false;
            math::mVec2 displacement{mFloat(m_playerHand.displacement.x),
                                     mFloat(m_playerHand.displacement.y)};

            for (auto entity : m_playerHand.hand)
            {
                m_componentManager.transforms[entity].position += displacement;
            }

            m_playerHand.displacement = {0, 0};
        }
    }

    void render(std::chrono::steady_clock::duration const& a_deltaTime)
    {
        m_end = std::chrono::high_resolution_clock::now();
        std::chrono::steady_clock::duration ddeltaTime = m_end - m_start;
        m_start = std::chrono::high_resolution_clock::now();

        mDouble deltaTime = std::chrono::duration<mDouble>(ddeltaTime).count();

        // Process the recorded gameActionsLists
        execute_gameActions(m_gameActions, m_componentManager);

        process_playerHand(m_gameActions);

        std::vector<std::pair<Entity, Entity>> collidingPairs;
        gather_intersectedObjects(m_completeCollisionDatas, collidingPairs);

        for (auto& pair : collidingPairs)
        {
            mLog_info("Intersected Entity : ", pair.first, " with ",
                      pair.second);
        }

        if (m_updateScene)
        {
            update_gameScene(ddeltaTime);
        }
        m_effectiveTransforms.resize(m_componentManager.entityCount);
        m_effectiveRenderingCpnts.resize(m_componentManager.entityCount);
        apply_animationModifiers(
            m_componentManager.animators, m_componentManager.transforms,
            m_componentManager.renderingCpnts, m_effectiveTransforms,
            m_effectiveRenderingCpnts);
        m_completeCollisionDatas.clear();

        process_collisions(m_effectiveTransforms, m_componentManager.collisions,
                           m_completeCollisionDatas);

        m_drawingData.clean_drawables();
        m_ranges.clear();
        m_meshBuffer.clear();
        process_renderableObjects(m_effectiveTransforms,
                                  m_effectiveRenderingCpnts, m_drawingData);

        static const mUInt indexPerQuad    = 5;
        static const mUInt vertexPerQuad   = 4;
        m::mU32            materialID      = 0;
        mUInt              totalNbDrawable = 0;
        for (auto drawables : m_drawingData.materialDrawables)
        {
            if (drawables.size() == 0)
            {
                ++materialID;
                continue;
            }

            m_ranges.emplace_back();
            m_ranges.back().materialID = materialID;
            m_ranges.back().indexCount = drawables.size() * indexPerQuad;
            m_ranges.back().indexStartLocation = totalNbDrawable * indexPerQuad;

            for (auto drawable : drawables)
            {
                generate_squareIntoMeshBuffer(&m_meshBuffer, drawable);
            }

            totalNbDrawable += drawables.size();
            ++materialID;
        }

        RECT clientRect = {};
        ::GetWindowRect(((win32::IWindowImpl*)(m_windowGame))->get_hwnd(),
                        &clientRect);

        math::mIVec2 targetPoint{0, 0};
        targetPoint.x = -clientRect.left - windowWidth / 2;
        targetPoint.y = clientRect.top - 1080 + windowHeight / 2;
        m_matrixGame  = math::generate_projectionOrthoLH(
                            screenWidth, -screenHeight, 0.0f, 1.0f) *
                       math::generate_translationMatrix(targetPoint.x,
                                                        targetPoint.y, 0.0f);

#ifdef ENABLE_EDITOR
        m_rangesEditor.clear();
        m_meshBufferEditor.clear();

        m_rangesEditor.emplace_back();
        m_rangesEditor.back().materialID         = 0;
        m_rangesEditor.back().indexCount         = 5 * indexPerQuad;
        m_rangesEditor.back().indexStartLocation = 0;

        generate_rectangleIntoMeshBuffer(
            &m_meshBufferEditor,
            {mFloat(clientRect.left), 1080 - mFloat(clientRect.top), 0.0f},
            {400, 300}, {1.0f, 1.0f, 1.0f, 0.1f});

        generate_rectangleIntoMeshBuffer(&m_meshBufferEditor,
                                         {0.0f, 0.0f, 0.0f}, {1920, -10},
                                         {0.0f, 0.0f, 0.0f, 0.1f});
        generate_rectangleIntoMeshBuffer(&m_meshBufferEditor,
                                         {0.0f, 0.0f, 0.0f}, {10, -1080},
                                         {0.0f, 0.0f, 0.0f, 0.1f});
        generate_rectangleIntoMeshBuffer(&m_meshBufferEditor,
                                         {1920.0f - 10.0f, 0.0f, 0.0f},
                                         {10, -1080}, {0.0f, 0.0f, 0.0f, 0.1f});
        generate_rectangleIntoMeshBuffer(&m_meshBufferEditor,
                                         {0.0f, 1080.0f, 0.0f}, {1920, 10},
                                         {0.0f, 0.0f, 0.0f, 0.1f});
        if (m_debugCollisions)
        {
            m_rangesEditor.emplace_back();
            m_rangesEditor.back().materialID = 0;
            m_rangesEditor.back().indexCount = draw_debugCollisions(
                m_completeCollisionDatas, m_meshBufferEditor);
            m_rangesEditor.back().indexStartLocation = 5 * indexPerQuad;
        }

        mInt screenWidth = m_pEditorSwapchain->get_desc().width;
        ;
        mInt screenHeight = m_pEditorSwapchain->get_desc().height;
        ;

        m_matrixEditor = math::generate_projectionOrthoLH(
                             screenWidth, screenHeight, 0.0f, 1.0f) *
                         m_targetController.m_worldToView;

        render_editorGUI();
#endif  // ENABLE_EDITOR

        m_tastsetExecutor.run();
    }

    mBool step(std::chrono::steady_clock::duration const& a_deltaTime) override
    {
        if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
        {
            return false;
        }

#ifdef ENABLE_EDITOR
        if (editorWindowIsDead)
        {
            return false;
        }
#endif  // ENABLE_EDITOR

        render(a_deltaTime);

        return true;
    }

    m::render::mTasksetExecutor m_tastsetExecutor;

    win32::IWindowImpl*     m_windowGame      = nullptr;
    m::render::mIApi*       m_pGameApi        = nullptr;
    m::render::mISwapchain* m_pGameSwapchain  = nullptr;
    m::render::mISynchTool* m_pGameSynchTool  = nullptr;
    render::Task2dRender*   m_pTaskRenderGame = nullptr;
    math::mMat4x4           m_matrixGame{};

    input::mCallbackInputManager m_inputManagerGame;
    PlayerHand                   m_playerHand;

    std::vector<resource::mRequestImage> m_imageRequested;

    std::vector<render::TaskData2dRender::mRange>     m_ranges;
    render::DataMeshBuffer<render::BasicVertex, mU16> m_meshBuffer;

    ComponentManager           m_componentManager;
    std::vector<TransformCpnt> m_effectiveTransforms;
    std::vector<RenderingCpnt> m_effectiveRenderingCpnts;
    std::vector<CollisionData> m_completeCollisionDatas;
    std::vector<GameAction*>   m_gameActions;

    DrawingData m_drawingData;

    ComponentManager m_scene;

    std::chrono::time_point<std::chrono::steady_clock> m_start;
    std::chrono::time_point<std::chrono::steady_clock> m_end;

    RECT m_initialClientRect{};

    mBool m_updateScene = true;

#ifdef ENABLE_EDITOR
    bool editorWindowIsDead = false;

    win32::IWindowImpl*     m_windowEditor            = nullptr;
    m::render::mIApi*       m_pEditorApi              = nullptr;
    m::render::mISwapchain* m_pEditorSwapchain        = nullptr;
    m::render::mISynchTool* m_pEditorSynchTool        = nullptr;
    render::Task2dRender*   m_pTaskRenderGameInEditor = nullptr;
    math::mMat4x4           m_matrixEditor{};

    input::mCallbackInputManager m_inputManagerEditor;
    mTargetController            m_targetController;

    // Editor specific rendering
    std::vector<resource::mRequestImage> m_imageRequestedEditor;

    std::vector<render::TaskData2dRender::mRange>     m_rangesEditor;
    render::DataMeshBuffer<render::BasicVertex, mU16> m_meshBufferEditor;

    mBool m_debugCollisions = false;
#endif  // ENABLE_EDITOR
};

M_EXECUTE_WINDOWED_APP(RendererTestApp)