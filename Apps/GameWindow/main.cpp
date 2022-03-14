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

class mPainter
{
   public:
    void add_paintedPosition(const math::mIVec2& a_position)
    {
        m_paintedPositions.push_back(
            math::mIVec2{mInt(a_position.x - screenWidth / 2),
                         mInt(screenHeight / 2 - a_position.y)});
    }

    std::vector<math::mIVec2> m_paintedPositions;
};

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
    void init(mCmdLine const& a_cmdLine, void* a_appData) override
    {
        crossPlatform::IWindowedApplication::init(a_cmdLine, a_appData);

        m_imageRequested.emplace_back();
        m_imageRequested.back().path = "data/textures/Mir.png";
        m_imageRequested.emplace_back();
        m_imageRequested.back().path = "data/textures/Character.png";
        m_imageRequested.emplace_back();
        m_imageRequested.back().path.resize(512);  // prep for imGui

        render::TaskData2dRender taskData_2dRender;
        taskData_2dRender.m_pMeshBuffer = &m_meshBuffer;
        taskData_2dRender.m_pRanges     = &m_ranges;

        m_iRendererVulkan = new vulkan::VulkanRenderer();
        m_iRendererVulkan->init();
        // Setup vulkan window
        m_windowVulkan = static_cast<win32::IWindowImpl*>(
            add_newWindow("Vulkan Window", screenWidth, screenHeight));
        m_windowVulkan->link_inputManager(&m_inputManager);
        m_hdlSurfaceVulkan = m_windowVulkan->link_renderer(m_iRendererVulkan);

        render::Taskset* taskset_renderPipelineVulkan =
            m_hdlSurfaceVulkan->surface->addNew_renderTaskset();

        taskData_2dRender.m_hdlOutput = m_hdlSurfaceVulkan;
        taskData_2dRender.m_pMatrix   = &m_vkMatrix;
        m_pVkTaskRender =
            (render::Task2dRender*)(taskData_2dRender.add_toTaskSet(
                taskset_renderPipelineVulkan));

        m_pVkTaskRender->add_texture(m_imageRequested[0]);
        m_pVkTaskRender->add_texture(m_imageRequested[1]);

        ((win32::IWindowImpl*)(m_windowVulkan))
            ->attach_toSpecialUpdate(mCallback(this, &RendererTestApp::render));
        ::GetWindowRect(m_windowVulkan->get_hwnd(), &m_initialClientRect);

        m_inputManager.attach_toMouseEvent(
            input::mMouseAction::mousePressed(input::mMouseButton::left),
            input::mMouseActionCallback(&m_painter,
                                        &mPainter::add_paintedPosition));

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

        m_start = std::chrono::high_resolution_clock::now();

        MONITORINFO monitorInfo = {};
        monitorInfo.cbSize      = sizeof(MONITORINFO);

        ::GetMonitorInfo(::MonitorFromWindow(m_windowVulkan->get_hwnd(),
                                             MONITOR_DEFAULTTONEAREST),
                         &monitorInfo);

        RECT clientRect = {};
        ::GetWindowRect(((win32::IWindowImpl*)(m_windowVulkan))->get_hwnd(),
                        &clientRect);
        windowWidth  = clientRect.right - clientRect.left;
        windowHeight = clientRect.bottom - clientRect.top;
        mInt xPos    = (monitorInfo.rcMonitor.right - windowWidth) / 2;
        mInt yPos    = (monitorInfo.rcMonitor.bottom - windowHeight) / 2;

        SetWindowPos(m_windowVulkan->get_hwnd(), NULL, xPos, yPos, windowWidth,
                     windowHeight, SWP_SHOWWINDOW | SWP_DRAWFRAME);

        Entity        mainCharacter = m_componentManager.create_entity();
        RenderingCpnt rCpnt;
        rCpnt.materialID  = 1;
        rCpnt.pictureSize = 32;
        rCpnt.color       = {1.0f, 1.0f, 1.0f, 1.0f};
        m_componentManager.enable_component(mainCharacter, rCpnt);
        TransformCpnt tCpnt;
        tCpnt.position = {1000, 600};
        tCpnt.angle    = 0;
        tCpnt.scale    = 2.0f;
        m_componentManager.enable_component(mainCharacter, tCpnt);
        AnimatorCpnt aCpnt;
        aCpnt.pAnimation = new Animation();
        aCpnt.pAnimation->animationDuration = std::chrono::seconds(2);
        auto& keys = aCpnt.pAnimation->keys;
        keys.resize(3);
        keys[0].advancement = 0;
        keys[1].advancement = 0.5;
        keys[2].advancement = 1;
        auto& modifiers = aCpnt.pAnimation->modifiers;
        modifiers.resize(3);
        modifiers[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        modifiers[0].scale = 0.5;
        modifiers[1].color = {1.0f, 1.0f, 1.0f, 1.0f};
        modifiers[1].scale = 2;
        modifiers[1].angle = 3.141592;
        modifiers[2].scale = 0.5;
        modifiers[2].color = {0.0f, 0.0f, 0.0f, 1.0f};
        m_componentManager.enable_component(mainCharacter, aCpnt);

        Entity secondaryCharacter = m_componentManager.create_entity();
        m_componentManager.enable_component(secondaryCharacter, rCpnt);
        tCpnt.position = {1500, 600};
        tCpnt.scale    = 1.0f;
        m_componentManager.enable_component(secondaryCharacter, tCpnt);

        secondaryCharacter = m_componentManager.create_entity();
        rCpnt.materialID   = 0;
        rCpnt.pictureSize  = 16;
        m_componentManager.enable_component(secondaryCharacter, rCpnt);
        tCpnt.position     = {0, 0};
        secondaryCharacter = m_componentManager.create_entity();
        m_componentManager.enable_component(secondaryCharacter, rCpnt);
        tCpnt.position = {0, -10};
        m_componentManager.enable_component(secondaryCharacter, tCpnt);
        secondaryCharacter = m_componentManager.create_entity();
        m_componentManager.enable_component(secondaryCharacter, rCpnt);
        tCpnt.position = {30, 0};
        m_componentManager.enable_component(secondaryCharacter, tCpnt);
        secondaryCharacter = m_componentManager.create_entity();
        m_componentManager.enable_component(secondaryCharacter, rCpnt);
        tCpnt.position = {0, 30};
        m_componentManager.enable_component(secondaryCharacter, tCpnt);
        secondaryCharacter = m_componentManager.create_entity();
        m_componentManager.enable_component(secondaryCharacter, rCpnt);
        tCpnt.position = {-10, 0};
        m_componentManager.enable_component(secondaryCharacter, tCpnt);
    }

    void destroy() override
    {
        crossPlatform::IWindowedApplication::destroy();

        m_iRendererVulkan->destroy();
        delete m_iRendererVulkan;

        dearImGui::destroy();
    }

    void render(std::chrono::steady_clock::duration const& a_deltaTime)
    {
        m_end = std::chrono::high_resolution_clock::now();
        std::chrono::steady_clock::duration ddeltaTime = m_end - m_start;
        m_start = std::chrono::high_resolution_clock::now();

        mDouble deltaTime = std::chrono::duration<mDouble>(ddeltaTime).count();

        m_drawingData.clean_drawables();
        m_ranges.clear();
        m_meshBuffer.clear();
        process_animatedObjects(m_componentManager, ddeltaTime);
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
        ::GetWindowRect(((win32::IWindowImpl*)(m_windowVulkan))->get_hwnd(),
                        &clientRect);
        float addPosx = m_initialClientRect.left - clientRect.left;
        float addPosy = m_initialClientRect.top - clientRect.top;

        m_targetController.set_target({-clientRect.left, clientRect.top});

        m_vkMatrix = math::generate_projectionOrthoLH(
                         screenWidth, -screenHeight, 0.0f, 1.0f) *
                     m_targetController.m_worldToView;

        if (m_hdlSurfaceVulkan->isValid)
        {
            m_hdlSurfaceVulkan->surface->render();
        }
    }

    mBool step(std::chrono::steady_clock::duration const& a_deltaTime) override
    {
        if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
        {
            return false;
        }

        render(a_deltaTime);

        return true;
    }

    m::render::IRenderer*       m_iRendererVulkan;
    m::render::ISurface::HdlPtr m_hdlSurfaceVulkan;
    win32::IWindowImpl*         m_windowVulkan = nullptr;

    render::Task2dRender*                             m_pVkTaskRender = nullptr;
    std::vector<resource::mRequestImage>              m_imageRequested;
    math::mMat4x4                                     m_vkMatrix{};
    std::vector<render::TaskData2dRender::mRange>     m_ranges;
    render::DataMeshBuffer<render::BasicVertex, mU16> m_meshBuffer;

    mPainter                     m_painter;
    mTargetController            m_targetController;
    input::mCallbackInputManager m_inputManager;

    ComponentManager m_componentManager;
    DrawingData      m_drawingData;

    std::chrono::time_point<std::chrono::steady_clock> m_start;
    std::chrono::time_point<std::chrono::steady_clock> m_end;

    RECT m_initialClientRect{};
};

M_EXECUTE_WINDOWED_APP(RendererTestApp)