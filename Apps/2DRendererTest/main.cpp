#include <Kernel/File.hpp>
#include <Kernel/Math.hpp>
#include <Kernel/MatHelpers.hpp>
#include <MesumCore/Kernel/Image.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/DX12Renderer/DX12Context.hpp>
#include <MesumGraphics/DearImgui/MesumDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTask2DRender.hpp>
#include <MesumGraphics/RenderTasks/RenderTaskDearImGui.hpp>
#include <MesumGraphics/Resources/Texture.hpp>
#include <MesumGraphics/VulkanRenderer/VulkanContext.hpp>

using namespace m;

static const mInt screenWidth  = 1280;
static const mInt screenHeight = 720;

math::mXoRandomNumberGenerator g_randomGenerator(0);

void add_square(render::DataMeshBuffer<render::BasicVertex, mU16>* a_meshBuffer,
                math::mVec2 const                                  a_position)
{
    mSoftAssert(a_meshBuffer != nullptr);

    mUInt               index = a_meshBuffer->m_vertices.size();
    mFloat              size  = 16;
    render::BasicVertex vertex;
    vertex.color    = {1.0f, 1.0f, 1.0f, 1.0f};
    vertex.position = {a_position.x - size, a_position.y - size, 0.5f};
    vertex.uv       = {0.0f, 0.0f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position = {a_position.x - size, a_position.y + size, 0.5f};
    vertex.uv       = {0.0f, 1.0f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position = {a_position.x + size, a_position.y - size, 0.5f};
    vertex.uv       = {1.0f, 0.0f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position = {a_position.x + size, a_position.y + size, 0.5f};
    vertex.uv       = {1.0f, 1.0f};
    a_meshBuffer->m_vertices.push_back(vertex);

    a_meshBuffer->m_indices.push_back(index);
    a_meshBuffer->m_indices.push_back(index + 1);
    a_meshBuffer->m_indices.push_back(index + 2);
    a_meshBuffer->m_indices.push_back(index + 3);
    a_meshBuffer->m_indices.push_back(0xFFFF);
}

struct Drawer_2D
{
    void add_square(math::mVec2 const a_position)
    {
        ::add_square(&m_meshBuffer, a_position);
    }

    void reset() { m_meshBuffer.clear(); }

    render::DataMeshBuffer<render::BasicVertex, mU16> m_meshBuffer;
};

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
        newPosition.y = screenHeight - a_position.y;
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
        m_paintedPositions.push_back(a_position);
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
            m_targetPoint.x -= a_position.x;
            m_targetPoint.y += a_position.y;
        }
    }

    mBool        m_middleButtonPressed = false;
    math::mIVec2 m_targetPoint{0, 0};
};

class RendererTestApp : public m::crossPlatform::IWindowedApplication
{
    void init(mCmdLine const& a_cmdLine, void* a_appData) override
    {
        crossPlatform::IWindowedApplication::init(a_cmdLine, a_appData);
        m_iRendererDx12   = new dx12::DX12Renderer();
        m_iRendererVulkan = new vulkan::VulkanRenderer();
        m_iRendererDx12->init();
        m_iRendererVulkan->init();

        // SetupDx12 Window
        m_windowDx12 = add_newWindow("Dx12 Window", screenWidth, screenHeight);
        m_windowDx12->link_inputManager(&m_inputManager);
        m_hdlSurfaceDx12 = m_windowDx12->link_renderer(m_iRendererDx12);

        dearImGui::init(*m_windowDx12);

        render::Taskset* taskset_renderPipelineDx12 =
            m_hdlSurfaceDx12->surface->addNew_renderTaskset();

        render::TaskData2dRender taskData_2dRender;
        taskData_2dRender.m_hdlOutput   = m_hdlSurfaceDx12;
        taskData_2dRender.m_pMeshBuffer = &m_drawer2d.m_meshBuffer;
        taskData_2dRender.m_pMatrix     = &m_matrix;
        taskData_2dRender.m_pRanges     = &m_ranges;
        m_pTaskRender = (render::Task2dRender*)(taskData_2dRender.add_toTaskSet(
            taskset_renderPipelineDx12));

        render::TaskDataDrawDearImGui taskData_drawDearImGui;
        taskData_drawDearImGui.m_hdlOutput = m_hdlSurfaceDx12;
        taskData_drawDearImGui.add_toTaskSet(taskset_renderPipelineDx12);

        //        m_inputManager.attach_toKeyEvent(
        //            input::mKeyAction::keyPressed(input::keyN),
        //            mCallback<void>(&m_bunchOfSquares,
        //            &BunchOfSquares::add_newSquare));

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

        // Setup vulkan window
        m_windowVulkan =
            add_newWindow("Vulkan Window", screenWidth, screenHeight);
        m_hdlSurfaceVulkan = m_windowVulkan->link_renderer(m_iRendererVulkan);

        render::Taskset* taskset_renderPipelineVulkan =
            m_hdlSurfaceVulkan->surface->addNew_renderTaskset();

        taskData_2dRender.m_hdlOutput   = m_hdlSurfaceVulkan;
        taskData_2dRender.m_pMeshBuffer = &m_drawer2d.m_meshBuffer;
        taskData_2dRender.add_toTaskSet(taskset_renderPipelineVulkan);

        m_imageRequested.emplace_back();
        m_imageRequested.back().path = "data/textures/Test.png";
        auto [msg, image] = resource::load_image(m_imageRequested.back());
        mAssert(mIsSuccess(msg));

        m_imageRequested.emplace_back();
        m_imageRequested.back().path = "data/textures/Test2.png";
        m_pTaskRender->add_texture(m_imageRequested.back());

        m_imageRequested.emplace_back();
        m_imageRequested.back().path.resize(512);  // prep for imGui

        //        render::ManagerTexture managerTexture;
        //        GpuTextureBank TextureBankDx12 =
        //        managerTexture.link_renderer(rendererDx12); GpuTextureBank
        //        TextureBankVulkan =
        //        managerTexture.link_renderer(rendererVulkan);
        //
        //        HdlTexture hdl = managerTexture.create_handle();
        //
        //        managerTexture.install_metaData(hdl, image);
        //
        //        TextureBankDx12.upload(hdl);
        //        TextureBankVulkan.upload(hdl);
    }

    void destroy() override
    {
        crossPlatform::IWindowedApplication::destroy();

        m_iRendererDx12->destroy();
        delete m_iRendererDx12;

        m_iRendererVulkan->destroy();
        delete m_iRendererVulkan;

        dearImGui::destroy();
    }

    mBool step(std::chrono::steady_clock::duration const& a_deltaTime) override
    {
        if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
        {
            return false;
        }

        mDouble deltaTime = std::chrono::duration<mDouble>(a_deltaTime).count();

        static math::mIVec2 lastPosRegPos;
        static math::mIVec2 lastPlaced;
        for (auto position : m_painter.m_paintedPositions)
        {
            lastPosRegPos = position;
            lastPlaced = math::mIVec2{position.x, screenHeight - position.y} +
                         m_targetController.m_targetPoint;
            m_bunchOfSquares.add_oneNewSquare(
                math::mIVec2{position.x, screenHeight - position.y} +
                m_targetController.m_targetPoint);
        }
        m_painter.m_paintedPositions.clear();

        m_drawer2d.reset();
        m_ranges.clear();
        static const mUInt indexPerQuad  = 5;
        static const mUInt vertexPerQuad = 4;

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
                m_drawer2d.add_square(positions[i]);
            }
            totalNbPositions += positions.size();
        }

        m_matrix =
            math::generate_projectionOrthoLH(screenWidth, -screenHeight, 0.0f,
                                             1.0f) *
            math::generate_translation(
                mFloat(-screenWidth / 2 - m_targetController.m_targetPoint.x),
                mFloat(-screenHeight / 2 + m_targetController.m_targetPoint.y),
                0.0f);

        start_dearImGuiNewFrame(m_iRendererDx12);

        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(),
                                     ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::Begin("Engine");
        {
            ImGui::Text("frame time : %f", deltaTime);
            ImGui::Text("frame FPS : %f", 1.0 / deltaTime);
            ImGui::Text("nbSuqares : %llu", totalNbPositions);
            ImGui::Text("Camera offset : %dx;%dy",
                        m_targetController.m_targetPoint.x,
                        m_targetController.m_targetPoint.y);
            ImGui::Text("Last clicked position : %dx;%dy", lastPosRegPos.x,
                        lastPosRegPos.y);
            ImGui::Text("Last placed position : %dx;%dy", lastPlaced.x,
                        lastPlaced.y);
        }
        ImGui::End();

        ImGui::Begin("Resource List");
        {
            for (mUInt i = 0; i < m_imageRequested.size() - 1; ++i)
            {
                if (ImGui::Button(m_imageRequested[i].path.c_str()))
                {
                    m_bunchOfSquares.currentMaterialID = i;
                }
            }
        }
        ImGui::End();

        ImGui::Begin("Material");
        {
            ImGui::InputText("path", m_imageRequested.back().path.data(),
                             m_imageRequested.back().path.capacity());

            if (ImGui::Button("Add Image"))
            {
                if (m_pTaskRender->add_texture(m_imageRequested.back()))
                {
                    m_imageRequested.emplace_back();
                    m_imageRequested.back().path.resize(512);
                }
            }
        }
        ImGui::End();

        ImGui::Render();

        if (m_hdlSurfaceDx12->isValid)
        {
            m_hdlSurfaceDx12->surface->render();
        }
        if (m_hdlSurfaceVulkan->isValid)
        {
            m_hdlSurfaceVulkan->surface->render();
        }

        return true;
    }

    m::render::IRenderer*       m_iRendererDx12;
    m::render::ISurface::HdlPtr m_hdlSurfaceDx12;
    windows::mIWindow*          m_windowDx12 = nullptr;

    m::render::IRenderer*       m_iRendererVulkan;
    m::render::ISurface::HdlPtr m_hdlSurfaceVulkan;
    windows::mIWindow*          m_windowVulkan = nullptr;

    render::Task2dRender*                         m_pTaskRender = nullptr;
    std::vector<resource::mRequestImage>          m_imageRequested;
    math::mMat4x4                                 m_matrix{};
    std::vector<render::TaskData2dRender::mRange> m_ranges;

    Drawer_2D                    m_drawer2d;
    mPainter                     m_painter;
    mTargetController            m_targetController;
    BunchOfSquares               m_bunchOfSquares;
    input::mCallbackInputManager m_inputManager;
};

M_EXECUTE_WINDOWED_APP(RendererTestApp)