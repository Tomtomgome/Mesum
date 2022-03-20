#include <Kernel/File.hpp>
#include <Kernel/Math.hpp>
#include <Kernel/MatHelpers.hpp>
#include <MesumCore/Kernel/Image.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/DX12Renderer/DX12Context.hpp>
#include <MesumGraphics/DearImgui/MesumDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTask2DRender.hpp>
#include <MesumGraphics/RenderTasks/RenderTaskDearImGui.hpp>
#include <MesumGraphics/VulkanRenderer/VulkanContext.hpp>

#include "Scene.hpp"

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

class RendererTestApp : public m::crossPlatform::IWindowedApplication
{
    void init_game()
    {
        m_iRendererVulkan = new vulkan::VulkanRenderer();
        m_iRendererVulkan->init();

        // Setup game window
        m_windowGame = static_cast<win32::IWindowImpl*>(
            add_newWindow("Game", screenWidth, screenHeight, true));
        m_hdlSurfaceGame = m_windowGame->link_renderer(m_iRendererVulkan);

        render::Taskset* taskset_renderPipelineGame =
            m_hdlSurfaceGame->surface->addNew_renderTaskset();

        render::TaskData2dRender taskData_2dRender;
        taskData_2dRender.m_pMeshBuffer = &m_meshBuffer;
        taskData_2dRender.m_pRanges     = &m_ranges;
        taskData_2dRender.m_hdlOutput   = m_hdlSurfaceGame;
        taskData_2dRender.m_pMatrix     = &m_matrixGame;
        m_pTaskRenderGame =
            (render::Task2dRender*)(taskData_2dRender.add_toTaskSet(
                taskset_renderPipelineGame));

        m_pTaskRenderGame->add_texture(m_imageRequested[0]);
        m_pTaskRenderGame->add_texture(m_imageRequested[1]);

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

        m_start = std::chrono::high_resolution_clock::now();
    }

    void init_editor()
    {
#ifdef ENABLE_EDITOR
        m_updateScene = false;

        m_imageRequestedEditor.emplace_back();
        m_imageRequestedEditor.back().path = "data/textures/Editor/Overlay.png";

        m_iRendererDx12 = new dx12::DX12Renderer();
        m_iRendererDx12->init();
        // Setup editor window
        m_windowEditor = static_cast<win32::IWindowImpl*>(
            add_newWindow("Editor", 1280, 720, false));
        m_windowEditor->link_inputManager(&m_inputManager);
        m_hdlSurfaceEditor = m_windowEditor->link_renderer(m_iRendererDx12);

        dearImGui::init(*m_windowEditor);

        render::Taskset* taskset_renderPipelineEditor =
            m_hdlSurfaceEditor->surface->addNew_renderTaskset();

        render::TaskData2dRender taskData_2dRender;
        taskData_2dRender.m_pMeshBuffer = &m_meshBuffer;
        taskData_2dRender.m_pRanges     = &m_ranges;
        taskData_2dRender.m_hdlOutput   = m_hdlSurfaceEditor;
        taskData_2dRender.m_pMatrix     = &m_matrixEditor;
        m_pTaskRenderGameInEditor =
            (render::Task2dRender*)(taskData_2dRender.add_toTaskSet(
                taskset_renderPipelineEditor));
        m_pTaskRenderGameInEditor->add_texture(m_imageRequested[0]);
        m_pTaskRenderGameInEditor->add_texture(m_imageRequested[1]);

        taskData_2dRender.m_pMeshBuffer = &m_meshBufferEditor;
        taskData_2dRender.m_pRanges     = &m_rangesEditor;
        render::Task2dRender* pTaskRenderEditor =
            (render::Task2dRender*)(taskData_2dRender.add_toTaskSet(
                taskset_renderPipelineEditor));
        pTaskRenderEditor->add_texture(m_imageRequestedEditor[0]);

        render::TaskDataDrawDearImGui taskData_drawDearImGui;
        taskData_drawDearImGui.m_hdlOutput = m_hdlSurfaceEditor;
        taskData_drawDearImGui.add_toTaskSet(taskset_renderPipelineEditor);

        m_inputManager.attach_toMouseEvent(
            input::mMouseAction::mousePressed(input::mMouseButton::middle),
            input::mMouseActionCallback(&m_targetController,
                                        &mTargetController::activate_moveMode));
        m_inputManager.attach_toMouseEvent(
            input::mMouseAction::mouseReleased(input::mMouseButton::middle),
            input::mMouseActionCallback(
                &m_targetController, &mTargetController::deactivate_moveMode));
        m_inputManager.attach_toMouseMoveEvent(input::mMouseActionCallback(
            &m_targetController, &mTargetController::move_target));

        m_inputManager.attach_toMouseScrollEvent(input::mScrollCallback(
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

        init_editor();
        init_game();

        // Setup entity system
        Entity        mainCharacter = m_scene.create_entity();
        RenderingCpnt rCpnt;
        rCpnt.materialID  = 1;
        rCpnt.pictureSize = 32;
        rCpnt.color       = {1.0f, 1.0f, 1.0f, 1.0f};
        m_scene.enable_component(mainCharacter, rCpnt);
        TransformCpnt tCpnt;
        tCpnt.position = {1000, 600};
        tCpnt.angle    = 0;
        tCpnt.scale    = 2.0f;
        m_scene.enable_component(mainCharacter, tCpnt);
        AnimatorCpnt aCpnt;
        aCpnt.pAnimation                    = new Animation();
        aCpnt.pAnimation->animationDuration = std::chrono::seconds(2);
        auto& keys                          = aCpnt.pAnimation->keys;
        keys.resize(3);
        keys[0].advancement = 0;
        keys[1].advancement = 0.5;
        keys[2].advancement = 1;
        auto& modifiers     = aCpnt.pAnimation->modifiers;
        modifiers.resize(3);
        modifiers[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        modifiers[0].scale = 0.5;
        modifiers[1].color = {1.0f, 1.0f, 1.0f, 1.0f};
        modifiers[1].scale = 2;
        modifiers[1].angle = 3.141592;
        modifiers[2].scale = 0.5;
        modifiers[2].color = {0.0f, 0.0f, 0.0f, 1.0f};
        m_scene.enable_component(mainCharacter, aCpnt);

        Entity secondaryCharacter = m_scene.create_entity();
        m_scene.enable_component(secondaryCharacter, rCpnt);
        tCpnt.position = {1500, 600};
        tCpnt.scale    = 1.0f;
        m_scene.enable_component(secondaryCharacter, tCpnt);

        m_componentManager.load_fromCopy(m_scene);

        Entity bigRedFlag = m_componentManager.create_entity();
        m_componentManager.enable_component(secondaryCharacter, rCpnt);
        tCpnt.position = {1200, 600};
        tCpnt.scale    = 5.0f;
        m_componentManager.enable_component(secondaryCharacter, tCpnt);
    }

    void destroy() override
    {
        crossPlatform::IWindowedApplication::destroy();

        m_scene.reset();
        m_componentManager.reset();

        m_iRendererVulkan->destroy();
        delete m_iRendererVulkan;

#ifdef ENABLE_EDITOR
        m_iRendererDx12->destroy();
        delete m_iRendererDx12;
#endif  // ENABLE_EDITOR

        dearImGui::destroy();
    }
#ifdef ENABLE_EDITOR
    void render_editorGUI()
    {
        start_dearImGuiNewFrame(m_iRendererDx12);

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
            ImGui::Checkbox("Allow Update", &m_updateScene);

            if (ImGui::Button("Reset cene"))
            {
                m_componentManager.reset();
            }
        }
        ImGui::End();

        ImGui::Begin("Level Browser");
        {
            std::filesystem::path currentPath = std::filesystem::current_path();
            std::filesystem::path levelPath{currentPath/"data"/"levels"};
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

        ImGui::Begin("Level Editor");
        {
            static char path[512] = "";
            ImGui::InputText("Level Path : ", path, 512);

            if (ImGui::Button("Save as"))
            {
                m_componentManager.save_toFile(path);
            }

            if (ImGui::CollapsingHeader("Entities"))
            {
                m_componentManager.display_gui();
            }
        }
        ImGui::End();

        ImGui::Render();
    }
#endif  // ENABLE_EDITOR

    void update_gameScene(
        std::chrono::steady_clock::duration const& a_deltaTime)
    {
        process_animatedObjects(m_componentManager, a_deltaTime);
    }

    void render(std::chrono::steady_clock::duration const& a_deltaTime)
    {
        m_end = std::chrono::high_resolution_clock::now();
        std::chrono::steady_clock::duration ddeltaTime = m_end - m_start;
        m_start = std::chrono::high_resolution_clock::now();

        mDouble deltaTime = std::chrono::duration<mDouble>(ddeltaTime).count();

        if (m_updateScene)
        {
            update_gameScene(ddeltaTime);
        }

        m_drawingData.clean_drawables();
        m_ranges.clear();
        m_meshBuffer.clear();
        process_renderableObjects(m_componentManager, m_drawingData);

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
        float addPosx = m_initialClientRect.left - clientRect.left;
        float addPosy = m_initialClientRect.top - clientRect.top;

        math::mIVec2 targetPoint{0, 0};
        targetPoint.x = -clientRect.left - windowWidth / 2;
        targetPoint.y = clientRect.top - 1080 + windowHeight / 2;
        m_matrixGame  = math::generate_projectionOrthoLH(
                            screenWidth, -screenHeight, 0.0f, 1.0f) *
                       math::generate_translationMatrix(targetPoint.x,
                                                        targetPoint.y, 0.0f);

        if (m_hdlSurfaceGame->isValid)
        {
            m_hdlSurfaceGame->surface->render();
        }

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
                                         {1.0f, 1.0f, 1.0f, 0.1f});
        generate_rectangleIntoMeshBuffer(&m_meshBufferEditor,
                                         {0.0f, 0.0f, 0.0f}, {10, -1080},
                                         {1.0f, 1.0f, 1.0f, 0.1f});
        generate_rectangleIntoMeshBuffer(&m_meshBufferEditor,
                                         {1920.0f - 10.0f, 0.0f, 0.0f},
                                         {10, -1080}, {1.0f, 1.0f, 1.0f, 0.1f});
        generate_rectangleIntoMeshBuffer(&m_meshBufferEditor,
                                         {0.0f, 1080.0f, 0.0f}, {1920, 10},
                                         {1.0f, 1.0f, 1.0f, 0.1f});

        m_matrixEditor =
            math::generate_projectionOrthoLH(1280, 720, 0.0f, 1.0f) *
            m_targetController.m_worldToView;

        render_editorGUI();

        if (m_hdlSurfaceEditor->isValid)
        {
            m_hdlSurfaceEditor->surface->render();
        }
#endif  // ENABLE_EDITOR
    }

    mBool step(std::chrono::steady_clock::duration const& a_deltaTime) override
    {
        if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
        {
            return false;
        }

        render(a_deltaTime);

#ifdef ENABLE_EDITOR
        if (!m_hdlSurfaceEditor->isValid)
        {
            return false;
        }
#endif  // ENABLE_EDITOR

        return true;
    }

    render::IRenderer*       m_iRendererVulkan;
    win32::IWindowImpl*      m_windowGame = nullptr;
    render::ISurface::HdlPtr m_hdlSurfaceGame;
    render::Task2dRender*    m_pTaskRenderGame = nullptr;
    math::mMat4x4            m_matrixGame{};

    std::vector<resource::mRequestImage> m_imageRequested;

    std::vector<render::TaskData2dRender::mRange>     m_ranges;
    render::DataMeshBuffer<render::BasicVertex, mU16> m_meshBuffer;

    input::mCallbackInputManager m_inputManager;

    ComponentManager m_componentManager;
    DrawingData      m_drawingData;

    ComponentManager m_scene;

    std::chrono::time_point<std::chrono::steady_clock> m_start;
    std::chrono::time_point<std::chrono::steady_clock> m_end;

    RECT m_initialClientRect{};

    mBool m_updateScene = true;

#ifdef ENABLE_EDITOR
    render::IRenderer*       m_iRendererDx12;
    win32::IWindowImpl*      m_windowEditor = nullptr;
    render::ISurface::HdlPtr m_hdlSurfaceEditor;
    render::Task2dRender*    m_pTaskRenderGameInEditor = nullptr;
    math::mMat4x4            m_matrixEditor{};

    mTargetController m_targetController;

    // Editor specific rendering
    std::vector<resource::mRequestImage> m_imageRequestedEditor;

    std::vector<render::TaskData2dRender::mRange>     m_rangesEditor;
    render::DataMeshBuffer<render::BasicVertex, mU16> m_meshBufferEditor;
#endif  // ENABLE_EDITOR
};

M_EXECUTE_WINDOWED_APP(RendererTestApp)