#include <Kernel/File.hpp>
#include <Kernel/Math.hpp>
#include <MesumCore/Kernel/Image.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/DX12Renderer/DX12Context.hpp>
#include <MesumGraphics/DearImgui/MesumDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTask2DRender.hpp>
#include <MesumGraphics/RenderTasks/RenderTaskDearImGui.hpp>
#include <MesumGraphics/Resources/Texture.hpp>
#include <MesumGraphics/VulkanRenderer/VulkanContext.hpp>

using namespace m;

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
    vertex.uv       = {0.0f, 1.0f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position = {a_position.x - size, a_position.y + size, 0.5f};
    vertex.uv       = {0.0f, 0.0f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position = {a_position.x + size, a_position.y - size, 0.5f};
    vertex.uv       = {1.0f, 1.0f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position = {a_position.x + size, a_position.y + size, 0.5f};
    vertex.uv       = {1.0f, 0.0f};
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
    void add_newSquare()
    {
        math::mVec2 newPosition;
        for (int i = 0; i < 100; i++)
        {
            newPosition.x = g_randomGenerator.get_nextFloat() * 1280;
            newPosition.y = g_randomGenerator.get_nextFloat() * 720;
            m_squarePositions.push_back(newPosition);
        }
    }

    void add_oneNewSquare(const math::mIVec2& a_position)
    {
        math::mVec2 newPosition;
        newPosition.x = a_position.x;
        newPosition.y = 720 - a_position.y;
        m_squarePositions.push_back(newPosition);
    }

    void update(const mDouble& a_deltaTime)
    {
        static mFloat time = 0.0;
        time += mFloat(a_deltaTime);
        for (auto& position : m_squarePositions)
        {
            position.x += std::sin(time * 10.0) * 0.001f;
            position.y += std::cos(time * 10.0) * 0.001f;
        }
    }

    std::vector<math::mVec2> m_squarePositions;
};

class RendererTestApp : public m::crossPlatform::IWindowedApplication
{
    void init(mCmdLine const& a_cmdLine, void* a_appData) override
    {
        crossPlatform::IWindowedApplication::init(a_cmdLine, a_appData);
        m_iRendererDx12 = new dx12::DX12Renderer();
        // m_iRendererVulkan = new vulkan::VulkanRenderer();
        m_iRendererDx12->init();
        // m_iRendererVulkan->init();

        // SetupDx12 Window
        m_windowDx12 = add_newWindow("Dx12 Window", 1280, 720);
        m_windowDx12->link_inputManager(&m_inputManager);
        m_hdlSurfaceDx12 = m_windowDx12->link_renderer(m_iRendererDx12);

        dearImGui::init(*m_windowDx12);

        render::Taskset* taskset_renderPipelineDx12 =
            m_hdlSurfaceDx12->surface->addNew_renderTaskset();

        render::TaskData2dRender taskData_2dRender;
        taskData_2dRender.m_hdlOutput   = m_hdlSurfaceDx12;
        taskData_2dRender.m_pMeshBuffer = &m_drawer2d.m_meshBuffer;
        taskData_2dRender.m_pMaterialID = &m_currentMatID;
        m_pTaskRender = (render::Task2dRender*)(taskData_2dRender.add_toTaskSet(
            taskset_renderPipelineDx12));

        render::TaskDataDrawDearImGui taskData_drawDearImGui;
        taskData_drawDearImGui.m_hdlOutput = m_hdlSurfaceDx12;
        taskData_drawDearImGui.add_toTaskSet(taskset_renderPipelineDx12);

        m_inputManager.attach_toKeyEvent(
            input::mKeyAction::keyPressed(input::keyN),
            mCallback<void>(&m_bunchOfSquares, &BunchOfSquares::add_newSquare));

        m_inputManager.attach_toMouseEvent(
            input::mMouseAction::mousePressed(input::mMouseButton::left),
            input::mMouseActionCallback(&m_bunchOfSquares,
                                        &BunchOfSquares::add_oneNewSquare));

        // Setup vulkan window
        //        m_windowVulkan     = add_newWindow("Vulkan Window", 1280,
        //        720); m_hdlSurfaceVulkan =
        //        m_windowVulkan->link_renderer(m_iRendererVulkan);
        //
        //        render::Taskset* taskset_renderPipelineVulkan =
        //            m_hdlSurfaceVulkan->surface->addNew_renderTaskset();

        //        taskData_2dRender.m_hdlOutput   = m_hdlSurfaceVulkan;
        //        taskData_2dRender.m_pMeshBuffer = &m_drawer2d.m_meshBuffer;
        //        taskData_2dRender.add_toTaskSet(taskset_renderPipelineVulkan);

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

        m_inputManager.attach_toKeyEvent(
            input::mKeyAction::keyPressed(input::keyN),
            mCallback<void>(&m_bunchOfSquares, &BunchOfSquares::add_newSquare));
    }

    void destroy() override
    {
        crossPlatform::IWindowedApplication::destroy();

        m_iRendererDx12->destroy();
        delete m_iRendererDx12;

        //        m_iRendererVulkan->destroy();
        //        delete m_iRendererVulkan;

        dearImGui::destroy();
    }

    mBool step(std::chrono::steady_clock::duration const& a_deltaTime) override
    {
        if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
        {
            return false;
        }

        mDouble deltaTime = std::chrono::duration<mDouble>(a_deltaTime).count();
        m_bunchOfSquares.update(deltaTime);

        m_drawer2d.reset();

        for (auto position : m_bunchOfSquares.m_squarePositions)
        {
            m_drawer2d.add_square(position);
        }

        start_dearImGuiNewFrame(m_iRendererDx12);

        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(),
                                     ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::Begin("Engine");
        {
            ImGui::Text("frame time : %f", deltaTime);
            ImGui::Text("frame FPS : %f", 1.0 / deltaTime);
            ImGui::Text("nbSuqares : %llu",
                        m_bunchOfSquares.m_squarePositions.size());
        }
        ImGui::End();

        ImGui::Begin("Resource List");
        {
            for (mUInt i = 0; i < m_imageRequested.size() - 1; ++i)
            {
                ImGui::Text("%d : %s", i, m_imageRequested[i].path.c_str());
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

            ImGui::SliderInt("Current Texture :", &m_currentMatID, 0,
                             m_imageRequested.size() - 2);
        }
        ImGui::End();

        ImGui::Render();

        if (m_hdlSurfaceDx12->isValid)
        {
            m_hdlSurfaceDx12->surface->render();
        }
        //        if (m_hdlSurfaceVulkan->isValid)
        //        {
        //            m_hdlSurfaceVulkan->surface->render();
        //        }

        return true;
    }

    m::render::IRenderer*       m_iRendererDx12;
    m::render::ISurface::HdlPtr m_hdlSurfaceDx12;
    windows::mIWindow*          m_windowDx12 = nullptr;

    //    m::render::IRenderer*       m_iRendererVulkan;
    //    m::render::ISurface::HdlPtr m_hdlSurfaceVulkan;
    //    windows::mIWindow*          m_windowVulkan = nullptr;

    render::Task2dRender*                m_pTaskRender = nullptr;
    std::vector<resource::mRequestImage> m_imageRequested;
    mInt                                 m_currentMatID = 0;

    Drawer_2D m_drawer2d;

    BunchOfSquares               m_bunchOfSquares;
    input::mCallbackInputManager m_inputManager;
};

M_EXECUTE_WINDOWED_APP(RendererTestApp)