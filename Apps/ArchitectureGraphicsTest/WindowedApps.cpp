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

        m_pDx12Api = new m::dx12::mApi();
        m_pDx12Api->init();
        auto& dx12Api = m::unref_safe(m_pDx12Api);

        m_pVulkanApi = new m::vulkan::mApi();
        m_pVulkanApi->init();
        auto& vulkanApi = m::unref_safe(m_pVulkanApi);

        m_mainDx12Window =
            add_newWindow("Cube mover app", width, height, false);

        m_mainVulkanWindow =
            add_newWindow("Cube mover app", width, height, false);

        m_tastsetExecutor.init();

        m::mUInt nbBackbuffer = 3;

        auto& dx12SynchTool   = m_pDx12Api->create_synchTool();
        m_pDx12SynchTool      = &dx12SynchTool;
        auto& vulkanSynchTool = m_pVulkanApi->create_synchTool();
        m_pVulkanSynchTool    = &vulkanSynchTool;

        m::render::mISynchTool::Desc desc{nbBackbuffer};
        dx12SynchTool.init(desc);
        vulkanSynchTool.init(desc);

        auto& dx12Swapchain = m_pDx12Api->create_swapchain();
        m_pDx12Swapchain    = &dx12Swapchain;
        m::render::init_swapchainWithWindow(*m_pDx12Api, m_tastsetExecutor,
                                            dx12Swapchain, dx12SynchTool,
                                            *m_mainDx12Window, nbBackbuffer);

        auto& vulkanSwapchain = m_pVulkanApi->create_swapchain();
        m_pVulkanSwapchain    = &vulkanSwapchain;
        m::render::init_swapchainWithWindow(*m_pVulkanApi, m_tastsetExecutor,
                                            vulkanSwapchain, vulkanSynchTool,
                                            *m_mainVulkanWindow, nbBackbuffer);

        // Dx12 window task setup
        m::dearImGui::init(*m_mainDx12Window);

        auto& dx12Taskset = m_pDx12Api->create_renderTaskset();

        m::render::mTaskDataSwapchainWaitForRT taskData_swapchainWaitForRT{};
        taskData_swapchainWaitForRT.pSwapchain = m_pDx12Swapchain;
        taskData_swapchainWaitForRT.pSynchTool = m_pDx12SynchTool;
        auto& dx12Task = static_cast<m::render::mTaskSwapchainWaitForRT&>(
            taskData_swapchainWaitForRT.add_toTaskSet(dx12Taskset));

        m::render::TaskDataDrawDearImGui taskData_drawDearImGui;
        taskData_drawDearImGui.pOutputRT = dx12Task.pOutputRT;
        taskData_drawDearImGui.nbFrames  = nbBackbuffer;
        taskData_drawDearImGui.add_toTaskSet(dx12Taskset);

        m::render::mTaskDataSwapchainPresent taskData_swapchainPresent{};
        taskData_swapchainPresent.pSwapchain = m_pDx12Swapchain;
        taskData_swapchainPresent.pSynchTool = m_pDx12SynchTool;
        taskData_swapchainPresent.add_toTaskSet(dx12Taskset);

        m_tastsetExecutor.confy_permanentTaskset(m::unref_safe(m_pDx12Api),
                                                 dx12Taskset);
        m_mainDx12Window->attach_toDestroy(m::mCallback<void>(
            [this, &dx12Api, &dx12Taskset]() {
                m_tastsetExecutor.remove_permanentTaskset(dx12Api, dx12Taskset);
            }));

        // Vulkan window task setup
        auto& vulkanTaskset = m_pVulkanApi->create_renderTaskset();

        taskData_swapchainWaitForRT = {};
        taskData_swapchainWaitForRT.pSwapchain = m_pVulkanSwapchain;
        taskData_swapchainWaitForRT.pSynchTool = m_pVulkanSynchTool;
        auto& vulkanTask = static_cast<m::render::mTaskSwapchainWaitForRT&>(
            taskData_swapchainWaitForRT.add_toTaskSet(vulkanTaskset));

//        taskData_drawDearImGui = {};
//        taskData_drawDearImGui.pOutputRT = task.pOutputRT;
//        taskData_drawDearImGui.nbFrames  = nbBackbuffer;
//        taskData_drawDearImGui.add_toTaskSet(vulkanTaskset);

        taskData_swapchainPresent = {};
        taskData_swapchainPresent.pSwapchain = m_pVulkanSwapchain;
        taskData_swapchainPresent.pSynchTool = m_pVulkanSynchTool;
        taskData_swapchainPresent.add_toTaskSet(vulkanTaskset);

        m_tastsetExecutor.confy_permanentTaskset(m::unref_safe(m_pVulkanApi),
                                                 vulkanTaskset);
        m_mainVulkanWindow->attach_toDestroy(m::mCallback<void>(
            [this, &vulkanApi, &vulkanTaskset]() {
                m_tastsetExecutor.remove_permanentTaskset(vulkanApi, vulkanTaskset);
            }));

        // Dx12 windo input setup
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

        m_tastsetExecutor.destroy();

        // call to destroy of the swapchain is managed at window termination
        m_pDx12Api->destroy_swapchain(*m_pDx12Swapchain);

        m_pDx12Api->destroy_synchTool(*m_pDx12SynchTool);

        m_pDx12Api->destroy();
        delete m_pDx12Api;

        m_pVulkanSwapchain->destroy();
        m_pVulkanApi->destroy_swapchain(*m_pVulkanSwapchain);

        m_pVulkanSynchTool->destroy();
        m_pVulkanApi->destroy_synchTool(*m_pVulkanSynchTool);

        m_pVulkanApi->destroy();
        delete m_pVulkanApi;

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
            m_tastsetExecutor.run();
        }

        return true;
    }

    m::mFloat m_x = 0.0f;
    m::mFloat m_y = 0.0f;

    m::render::mTasksetExecutor m_tastsetExecutor;

    m::render::mIApi*       m_pDx12Api;
    m::render::mISwapchain* m_pDx12Swapchain;
    m::render::mISynchTool* m_pDx12SynchTool;

    m::render::mIApi*       m_pVulkanApi;
    m::render::mISwapchain* m_pVulkanSwapchain;
    m::render::mISynchTool* m_pVulkanSynchTool;

    m::input::mCallbackInputManager m_inputManager;
    m::windows::mIWindow*           m_mainDx12Window;
    m::windows::mIWindow*           m_mainVulkanWindow;
    CubeMover                       m_mover;

    const m::logging::mChannelID m_CUBEAPP_ID = mLog_getId();
};

M_EXECUTE_WINDOWED_APP(CubeMoverApp)
