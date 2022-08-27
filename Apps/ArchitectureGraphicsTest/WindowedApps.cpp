#include <MesumCore/Kernel/Kernel.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/DearImgui/MesumDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTaskDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTasksBasicSwapchain.hpp>
#include <MesumGraphics/WindowedApp.hpp>
#include <MesumGraphics/ApiAbstraction.hpp>
#include <MesumGraphics/RendererUtils.hpp>

class CubeMover
{
   public:
    void move(m::mFloat& x, m::mFloat& y)
    {
        if (m_up)
        {
            y -= speed;
        }
        if (m_down)
        {
            y += speed;
        }
        if (m_left)
        {
            x -= speed;
        }
        if (m_right)
        {
            x += speed;
        }
        mLog_infoTo(m_CUBEMOVER_ID, "Se deplace : ", x, ":", y);
    }

    void set_moveUp() { m_up = true; }
    void set_moveDown() { m_down = true; }
    void set_moveLeft() { m_left = true; }
    void set_moveRight() { m_right = true; }

    void set_notMoveUp() { m_up = false; }
    void set_notMoveDown() { m_down = false; }
    void set_notMoveLeft() { m_left = false; }
    void set_notMoveRight() { m_right = false; }

   private:
    m::mFloat speed   = 0.016f * 100.f;
    m::mBool  m_up    = false;
    m::mBool  m_down  = false;
    m::mBool  m_left  = false;
    m::mBool  m_right = false;

    const m::logging::mChannelID m_CUBEMOVER_ID = mLog_getId();
};

void enumerate_adapter(m::aa::mApi&                  a_api,
                       std::vector<m::aa::mAdapter>& a_adapters)
{
}

class CubeMoverApp : public m::crossPlatform::IWindowedApplication
{
    void add_applicationWindow()
    {
        m::windows::mIWindow* m_secondWindow =
            add_newWindow("Test Second window", 600, 400, false);
        m_secondWindow->link_inputManager(&m_inputManager);
    }

    void init(m::mCmdLine const& a_cmdLine, void* a_appData) override
    {
        m::crossPlatform::IWindowedApplication::init(a_cmdLine, a_appData);

        m::mCmdLine const& cmdLine = a_cmdLine;
        m::mUInt           width   = 1280;
        m::mUInt           height  = 720;
        if (!cmdLine.get_parameter("-w", width))
        {
            mLog_infoTo(m_CUBEAPP_ID,
                        "Width not overridden, use default : ", width);
        }

        if (!cmdLine.get_parameter("-h", height))
        {
            mLog_infoTo(m_CUBEAPP_ID,
                        "Height not overridden, use default : ", height);
        }

        m_pDx12Api = new m::dx12::mApiDX12();
        m_pDx12Api->init();
        m_iVulkanRenderer = new m::vulkan::VulkanRenderer();
        m_iVulkanRenderer->init();

        m_mainDx12Window =
            add_newWindow("Cube mover app", width, height, false);

        m_mainVulkanWindow =
            add_newWindow("Cube mover app", width, height, false);

        m::mUInt nbBackbuffer = 3;

        auto& dx12Swapchain = m_pDx12Api->create_swapchain();
        m_pDx12Swapchain    = &dx12Swapchain;
        m::render::init_swapchainWithWindow(*m_pDx12Api, dx12Swapchain,
                                            *m_mainDx12Window, nbBackbuffer);

        m_hdlVulkanSurface =
            m_mainVulkanWindow->link_renderer(m_iVulkanRenderer);

        m::dearImGui::init(*m_mainDx12Window);

        auto& dx12SynchTool = m_pDx12Api->create_synchTool();
        m_pDx12SynchTool = &dx12SynchTool;
        m::render::mISynchTool::Desc desc{nbBackbuffer};
        dx12SynchTool.init(desc);

        auto& dx12Taskset = m_pDx12Api->create_renderTaskset();
        m_pDx12Taskset    = &dx12Taskset;

        m::render::mTaskDataSwapchainWaitForRT taskData_swapchainWaitForRT{};
        taskData_swapchainWaitForRT.pSwapchain = m_pDx12Swapchain;
        taskData_swapchainWaitForRT.pSynchTool  = m_pDx12SynchTool;
        auto& task = static_cast<m::render::mTaskSwapchainWaitForRT&>(
            taskData_swapchainWaitForRT.add_toTaskSet(dx12Taskset));

        m::render::TaskDataDrawDearImGui taskData_drawDearImGui;
        taskData_drawDearImGui.pOutputRT = task.pOutputRT;
        taskData_drawDearImGui.add_toTaskSet(dx12Taskset);

        m::render::mTaskDataSwapchainPresent taskData_swapchainPresent{};
        taskData_swapchainPresent.pSwapchain = m_pDx12Swapchain;
        taskData_swapchainPresent.pSynchTool  = m_pDx12SynchTool;
        taskData_swapchainPresent.add_toTaskSet(dx12Taskset);

        m_mainDx12Window->link_inputManager(&m_inputManager);

        m_inputManager.attach_toKeyEvent(
            m::input::mKeyAction::keyPressed(m::input::keyF11),
            m::input::mKeyActionCallback(
                m_mainDx12Window, &m::windows::mIWindow::toggle_fullScreen));

        m_inputManager.attach_toKeyEvent(
            m::input::mKeyAction::keyPressed(m::input::keyW),
            m::input::mKeyActionCallback(this,
                                         &CubeMoverApp::add_applicationWindow));

        m_inputManager.attach_toKeyEvent(
            m::input::mKeyAction::keyPressed(m::input::keyUp),
            m::input::mKeyActionCallback(&m_mover, &CubeMover::set_moveUp));
        m_inputManager.attach_toKeyEvent(
            m::input::mKeyAction::keyPressed(m::input::keyDown),
            m::input::mKeyActionCallback(&m_mover, &CubeMover::set_moveDown));
        m_inputManager.attach_toKeyEvent(
            m::input::mKeyAction::keyPressed(m::input::keyLeft),
            m::input::mKeyActionCallback(&m_mover, &CubeMover::set_moveLeft));
        m_inputManager.attach_toKeyEvent(
            m::input::mKeyAction::keyPressed(m::input::keyRight),
            m::input::mKeyActionCallback(&m_mover, &CubeMover::set_moveRight));

        m_inputManager.attach_toKeyEvent(
            m::input::mKeyAction::keyReleased(m::input::keyUp),
            m::input::mKeyActionCallback(&m_mover, &CubeMover::set_notMoveUp));
        m_inputManager.attach_toKeyEvent(
            m::input::mKeyAction::keyReleased(m::input::keyDown),
            m::input::mKeyActionCallback(&m_mover,
                                         &CubeMover::set_notMoveDown));
        m_inputManager.attach_toKeyEvent(
            m::input::mKeyAction::keyReleased(m::input::keyLeft),
            m::input::mKeyActionCallback(&m_mover,
                                         &CubeMover::set_notMoveLeft));
        m_inputManager.attach_toKeyEvent(
            m::input::mKeyAction::keyReleased(m::input::keyRight),
            m::input::mKeyActionCallback(&m_mover,
                                         &CubeMover::set_notMoveRight));

        set_minimalStepDuration(std::chrono::milliseconds(16));
    }

    void destroy() override
    {
        m::crossPlatform::IWindowedApplication::destroy();

        m_pDx12Taskset->clear();
        m_pDx12Api->destroy_renderTaskset(*m_pDx12Taskset);

        m_pDx12Swapchain->destroy();
        m_pDx12Api->destroy_swapchain(*m_pDx12Swapchain);

        m_pDx12Api->destroy();
        delete m_pDx12Api;

        m_iVulkanRenderer->destroy();
        delete m_iVulkanRenderer;

        m::dearImGui::destroy();
    }

    m::mBool step(
        std::chrono::steady_clock::duration const& a_deltaTime) override
    {
        if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
        {
            return false;
        }

        mLog_infoTo(m_CUBEAPP_ID, "Bonjour !, dt = ", a_deltaTime);

        m_mover.move(m_x, m_y);

        start_dearImGuiNewFrame(*m_pDx12Api);

        ImGui::NewFrame();

        m::mBool showDemo = true;
        ImGui::ShowDemoWindow(&showDemo);
        ImGui::Render();

        // TODO : if window still open
        if (m_mainDx12Window != nullptr)
        {
            m_pDx12Taskset->execute();
        }

        if (m_hdlVulkanSurface->isValid)
        {
            m_hdlVulkanSurface->surface->render();
        }

        return true;
    }

    m::mFloat m_x = 0.0f;
    m::mFloat m_y = 0.0f;

    m::render::mIApi*       m_pDx12Api;
    m::render::mISwapchain* m_pDx12Swapchain;
    m::render::mISynchTool* m_pDx12SynchTool;
    m::render::Taskset*     m_pDx12Taskset;

    m::render::IRenderer*           m_iVulkanRenderer;
    m::render::ISurface::HdlPtr     m_hdlVulkanSurface;
    m::input::mCallbackInputManager m_inputManager;
    m::windows::mIWindow*           m_mainDx12Window;
    m::windows::mIWindow*           m_mainVulkanWindow;
    CubeMover                       m_mover;

    const m::logging::mChannelID m_CUBEAPP_ID = mLog_getId();
};

M_EXECUTE_WINDOWED_APP(CubeMoverApp)
