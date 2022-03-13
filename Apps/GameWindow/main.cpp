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

using namespace m;

static const mInt screenWidth  = 400;
static const mInt screenHeight = 300;

math::mXoRandomNumberGenerator g_randomGenerator(0);

struct RenderingCpnt
{
    m::mU32 materialID;
};

struct TransformCpnt
{
    m::math::mVec2 position;
    m::mFloat      angle;
    m::mFloat      scale;
};

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
    GenerationData const&                              a_generationInfo)
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

struct Drawer_2D
{
    void add_square(TransformCpnt const& a_transform)
    {
        GenerationData data;
        data.color = {1.0f, 1.0f, 1.0f, 1.0f};
        data.offset = {a_transform.position.x, a_transform.position.y, 0.0f};
        data.size = /*a_transform.size*/ 32 * a_transform.scale;
        data.angle     = a_transform.angle;

        generate_squareIntoMeshBuffer(&m_meshBuffer, data);
    }

    void reset() { m_meshBuffer.clear(); }

    render::DataMeshBuffer<render::BasicVertex, mU16> m_meshBuffer;
};

// struct RenderBatches
//{
//     std::vector<std::vector<RenderingData>> m_squarePositions;
// };

struct BunchOfSquares
{
    void prepare_add()
    {
        if (m_squarePositions.size() < currentMaterialID + 1)
        {
            m_squarePositions.resize(currentMaterialID + 1);
        }
    }

    void add_newSquare()
    {
        prepare_add();

        math::mVec2 newPosition;
        for (int i = 0; i < 100; i++)
        {
            newPosition.x = g_randomGenerator.get_nextFloat() * screenWidth;
            newPosition.y = g_randomGenerator.get_nextFloat() * screenHeight;
            m_squarePositions[currentMaterialID].push_back(newPosition);
        }
    }

    void add_oneNewSquare(const math::mIVec2& a_position)
    {
        prepare_add();

        math::mVec2 newPosition;
        newPosition.x = a_position.x;
        newPosition.y = a_position.y;
        m_squarePositions[currentMaterialID].push_back(newPosition);
    }

    mInt                                  currentMaterialID = 0;
    std::vector<std::vector<math::mVec2>> m_squarePositions;
};

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
        m_imageRequested.back().path = "data/textures/Character.png";
        m_imageRequested.emplace_back();
        m_imageRequested.back().path = "data/textures/Test2.png";
        m_imageRequested.emplace_back();
        m_imageRequested.back().path.resize(512);  // prep for imGui

        render::TaskData2dRender taskData_2dRender;
        taskData_2dRender.m_pMeshBuffer = &m_drawer2d.m_meshBuffer;
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

        m_painter.add_paintedPosition(math::mIVec2{500, 500});

        m_start = std::chrono::high_resolution_clock::now();

        MONITORINFO monitorInfo = {};
        monitorInfo.cbSize      = sizeof(MONITORINFO);

        ::GetMonitorInfo(::MonitorFromWindow(m_windowVulkan->get_hwnd(),
                                             MONITOR_DEFAULTTONEAREST),
                         &monitorInfo);

        mInt xPos = (monitorInfo.rcMonitor.right - screenWidth) / 2;
        mInt yPos = (monitorInfo.rcMonitor.bottom - screenHeight) / 2;

        SetWindowPos(m_windowVulkan->get_hwnd(), NULL, xPos, yPos, screenWidth,
                     screenHeight, SWP_SHOWWINDOW | SWP_DRAWFRAME);
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

        static math::mIVec2 lastPosRegPos;
        static math::mIVec2 lastPlaced;
        mFloat              fact = 1.0 / m_targetController.m_zoomLevel;
        math::mVec4         floatPosition{0.0f, 0.0f, 0.0f, 1.0f};
        math::mVec4         transPosition{};
        for (auto position : m_painter.m_paintedPositions)
        {
            floatPosition.x = position.x;
            floatPosition.y = position.y;
            transPosition   = m_targetController.m_viewToWorld * floatPosition;
            lastPosRegPos   = position;
            lastPlaced =
                math::mIVec2{mInt(transPosition.x), mInt(transPosition.y)};
            m_bunchOfSquares.add_oneNewSquare(
                math::mIVec2{mInt(transPosition.x), mInt(transPosition.y)});
        }
        m_painter.m_paintedPositions.clear();

        m_drawer2d.reset();
        m_ranges.clear();
        static const mUInt indexPerQuad  = 5;
        static const mUInt vertexPerQuad = 4;

        static mFloat  addPos     = 0;
        static mDouble globalTime = 0;
        globalTime += deltaTime;
        addPos = 100.0f * std::sin(globalTime);

        static mFloat addAngle = 0;
        addAngle               = 2.0f * 3.141592 * std::sin(globalTime);

        RECT clientRect = {};
        ::GetWindowRect(((win32::IWindowImpl*)(m_windowVulkan))->get_hwnd(),
                        &clientRect);
        float addPosx          = m_initialClientRect.left - clientRect.left;
        float addPosy          = m_initialClientRect.top - clientRect.top;
        mUInt totalNbPositions = 0;
        for (mUInt j = 0; j < m_bunchOfSquares.m_squarePositions.size(); ++j)
        {
            auto& positions = m_bunchOfSquares.m_squarePositions[j];
            if (positions.size() == 0)
            {
                continue;
            }

            m_ranges.emplace_back();
            m_ranges.back().materialID = j;
            m_ranges.back().indexCount = positions.size() * indexPerQuad;
            m_ranges.back().indexStartLocation =
                totalNbPositions * indexPerQuad;
            for (mUInt i = 0; i < positions.size(); ++i)
            {
                auto modifPos =
                    positions[i] + math::mVec2{addPosx + addPos, -addPosy};

                TransformCpnt transform;
                transform.position = modifPos;
                transform.angle    = addAngle;
                transform.scale    = 1.0f;
                m_drawer2d.add_square(transform);
            }
            totalNbPositions += positions.size();
        }

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

    render::Task2dRender*                         m_pVkTaskRender = nullptr;
    std::vector<resource::mRequestImage>          m_imageRequested;
    math::mMat4x4                                 m_vkMatrix{};
    std::vector<render::TaskData2dRender::mRange> m_ranges;

    Drawer_2D                    m_drawer2d;
    mPainter                     m_painter;
    mTargetController            m_targetController;
    BunchOfSquares               m_bunchOfSquares;
    input::mCallbackInputManager m_inputManager;

    std::chrono::time_point<std::chrono::steady_clock> m_start;
    std::chrono::time_point<std::chrono::steady_clock> m_end;

    RECT m_initialClientRect{};
};

M_EXECUTE_WINDOWED_APP(RendererTestApp)