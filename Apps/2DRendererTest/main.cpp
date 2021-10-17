#include <File.hpp>
#include <Math.hpp>
#include <MesumCore/Kernel/Image.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/DX12Renderer/DX12Context.hpp>
#include <MesumGraphics/DearImgui/MesumDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTask2DRender.hpp>
#include <MesumGraphics/RenderTasks/RenderTaskDearImGui.hpp>
#include <MesumGraphics/Resources/Texture.hpp>
#include <MesumGraphics/VulkanRenderer/VulkanContext.hpp>

using namespace m;

math::RandomGenerator g_randomGenerator;

void add_squareToMesh(
    render::DataMeshBuffer<render::BasicVertex, U16>* a_meshBuffer,
    math::Vec2 const                                  a_position)
{
    mAssert(a_meshBuffer != nullptr);

    UInt                index = a_meshBuffer->m_vertices.size();
    Float               size  = 0.01;
    render::BasicVertex vertex;
    vertex.color    = {1.0f, 1.0f, 1.0f, 1.0f};
    vertex.position = {a_position.x - size, a_position.y - size, 0.5f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position = {a_position.x - size, a_position.y + size, 0.5f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position = {a_position.x + size, a_position.y - size, 0.5f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position = {a_position.x + size, a_position.y + size, 0.5f};
    a_meshBuffer->m_vertices.push_back(vertex);

    a_meshBuffer->m_indices.push_back(index);
    a_meshBuffer->m_indices.push_back(index + 1);
    a_meshBuffer->m_indices.push_back(index + 2);
    a_meshBuffer->m_indices.push_back(index + 3);
    a_meshBuffer->m_indices.push_back(0xFFFF);
}

struct Drawer_2D
{
    void add_square(math::Vec2 const a_position)
    {
        add_squareToMesh(&m_meshBuffer, a_position);
    }

    void reset() { m_meshBuffer.clear(); }

    render::DataMeshBuffer<render::BasicVertex, U16> m_meshBuffer;
};

struct BunchOfSquares
{
    void add_newSquare()
    {
        math::Vec2 newPosition;
        for (int i = 0; i < 100; i++)
        {
            newPosition.x = g_randomGenerator.get_nextFloat();
            newPosition.y = g_randomGenerator.get_nextFloat();
            m_squarePositions.push_back(newPosition);
        }
    }

    void update(const Double& a_deltaTime)
    {
        static Float time = 0.0;
        time += Float(a_deltaTime);
        for (auto& position : m_squarePositions)
        {
            position.x += std::sin(time * 10.0) * 0.001f;
            position.y += std::cos(time * 10.0) * 0.001f;
        }
    }

    std::vector<math::Vec2> m_squarePositions;
};

class RendererTestApp : public m::crossPlatform::IWindowedApplication
{
    void init() override
    {
        crossPlatform::IWindowedApplication::init();
        m_iRendererDx12   = new dx12::DX12Renderer();
        m_iRendererVulkan = new vulkan::VulkanRenderer();
        m_iRendererDx12->init();
        m_iRendererVulkan->init();

        g_randomGenerator.init(0);

        // SetupDx12 Window
        m_windowDx12 = add_newWindow("Dx12 Window", 1280, 720);
        m_windowDx12->link_inputManager(&m_inputManager);
        m_hdlSurfaceDx12 = m_windowDx12->link_renderer(m_iRendererDx12);
        m_windowDx12->set_asMainWindow();

        dearImGui::init(m_windowDx12);

        render::Taskset* taskset_renderPipelineDx12 =
            m_hdlSurfaceDx12->surface->addNew_renderTaskset();

        render::TaskData2dRender taskData_2dRender;
        taskData_2dRender.m_hdlOutput   = m_hdlSurfaceDx12;
        taskData_2dRender.m_pMeshBuffer = &m_drawer2d.m_meshBuffer;
        taskData_2dRender.add_toTaskSet(taskset_renderPipelineDx12);

        render::TaskDataDrawDearImGui taskData_drawDearImGui;
        taskData_drawDearImGui.m_hdlOutput = m_hdlSurfaceDx12;
        taskData_drawDearImGui.add_toTaskSet(taskset_renderPipelineDx12);

        m_inputManager.attach_ToKeyEvent(
            input::KeyAction::keyPressed(input::KEY_N),
            Callback<void>(&m_bunchOfSquares, &BunchOfSquares::add_newSquare));

        // Setup vulkan window
        m_windowVulkan     = add_newWindow("Vulkan Window", 1280, 720);
        m_hdlSurfaceVulkan = m_windowVulkan->link_renderer(m_iRendererVulkan);

        render::Taskset* taskset_renderPipelineVulkan =
            m_hdlSurfaceVulkan->surface->addNew_renderTaskset();

        taskData_2dRender.m_hdlOutput   = m_hdlSurfaceVulkan;
        taskData_2dRender.m_pMeshBuffer = &m_drawer2d.m_meshBuffer;
        taskData_2dRender.add_toTaskSet(taskset_renderPipelineVulkan);

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

        m_inputManager.attach_ToKeyEvent(
            input::KeyAction::keyPressed(input::KEY_N),
            Callback<void>(&m_bunchOfSquares, &BunchOfSquares::add_newSquare));
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

    Bool step(const Double& a_deltaTime) override
    {
        if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
        {
            return false;
        }

        m_bunchOfSquares.update(a_deltaTime);

        m_drawer2d.reset();

        for (auto position : m_bunchOfSquares.m_squarePositions)
        {
            m_drawer2d.add_square(position);
        }

        start_dearImGuiNewFrame(m_iRendererDx12);

        ImGui::NewFrame();
        ImGui::Begin("Engine");
        {
            ImGui::Text("frame time : %f", a_deltaTime);
            ImGui::Text("frame FPS : %f", 1.0 / a_deltaTime);
            ImGui::Text("nbSuqares : %llu",
                        m_bunchOfSquares.m_squarePositions.size());
        }
        ImGui::End();
        ImGui::Render();

        m_hdlSurfaceDx12->surface->render();
        if (m_hdlSurfaceVulkan->isValid)
        {
            m_hdlSurfaceVulkan->surface->render();
        }

        return true;
    }

    m::render::IRenderer*       m_iRendererDx12;
    m::render::ISurface::HdlPtr m_hdlSurfaceDx12;
    windows::IWindow*           m_windowDx12 = nullptr;

    m::render::IRenderer*       m_iRendererVulkan;
    m::render::ISurface::HdlPtr m_hdlSurfaceVulkan;
    windows::IWindow*           m_windowVulkan = nullptr;

    Drawer_2D m_drawer2d;

    BunchOfSquares      m_bunchOfSquares;
    input::InputManager m_inputManager;
};

M_EXECUTE_WINDOWED_APP(RendererTestApp)