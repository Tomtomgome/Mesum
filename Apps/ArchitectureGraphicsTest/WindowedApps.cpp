#include <MesumCore/Kernel/Kernel.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/DearImgui/MesumDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTaskDearImGui.hpp>
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
                        "Width not overriden, use default : ", width);
        }

        if (!cmdLine.get_parameter("-h", height))
        {
            mLog_infoTo(m_CUBEAPP_ID,
                        "Height not overriden, use default : ", height);
        }

        //        m::aa::mApi           api = m::aa::dx12::create_api();
        //        m::aa::mApi::InitData apiInitData;
        //        api.init(apiInitData);
        //        m::aa::mAdapter           a = api.create_adapter();
        //        mLink_virtualMemberFunctionEXT(api, enumerate_adapter,
        //        enumerate_adapter);
        //
        //        m::aa::mAdapter::InitData initData;
        //        a.init(initData);
        //
        //        std::vector<m::aa::mAdapter> adapters;
        //        api.enumerate_adapter(adapters);
        //
        //        m::aa::mAdapter selectedAdapter;
        //        for (auto& adapter : adapters)
        //        {
        //            if (adapter.extensions.supportsSwapChain)
        //            {
        //                continue;
        //            }
        //            if (adapter.features)
        //            {
        //                continue;
        //            }
        //            if (adapter.properties.type !=
        //            m::aa::mAdapter::deviceDescrete)
        //            {
        //                continue;
        //            }
        //
        //            selectedAdapter = adapter;
        //        }
        //
        //        m::aa::mDevice::InitData initData;
        //        initData.queues = {{mQueue::type::transfer graphics},
        //        {mQueue::type::transfer}}; m::aa::mDevice device =
        //        selectedAdapter.create_device(initData); m::aa::mQueue
        //        graphicsQueue = device.get_queue(0); m::aa::mQueue
        //        transferQueue = device.get_queue(1);
        //
        //        m::aa::mApi api2 = m::aa::vulkan::create_api();
        //        api2.init(apiInitData);
        //        m::aa::mAdapter a2 = api2.create_adapter();
        //        a2.init(initData);

        //m::render::mIApi* pDx12Api = new m::dx12::mApiDX12();
        //pDx12Api->init();
        m_iDx12Renderer = new m::dx12::DX12Renderer();
        m_iDx12Renderer->init();
        m_iVulkanRenderer = new m::vulkan::VulkanRenderer();
        m_iVulkanRenderer->init();

        m_mainDx12Window =
            add_newWindow("Cube mover app", width, height, false);

        m_mainVulkanWindow =
            add_newWindow("Cube mover app", width, height, false);

        m_hdlDx12Surface = m_mainDx12Window->link_renderer(m_iDx12Renderer);
        //auto& swapchain = pDx12Api->create_swapchain();
        //m::render::init_swapchainWithWindow(*pDx12Api, swapchain,
        //                                    *m_mainDx12Window);

        m_hdlVulkanSurface =
            m_mainVulkanWindow->link_renderer(m_iVulkanRenderer);

        m::dearImGui::init(*m_mainDx12Window);

        m::render::Taskset* taskset_renderPipeline =
            m_hdlDx12Surface->surface->addNew_renderTaskset();

        m::render::TaskDataDrawDearImGui taskData_drawDearImGui;
        taskData_drawDearImGui.m_hdlOutput = m_hdlDx12Surface;
        taskData_drawDearImGui.add_toTaskSet(taskset_renderPipeline);

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

        m_iDx12Renderer->destroy();
        delete m_iDx12Renderer;

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

        start_dearImGuiNewFrame(m_iDx12Renderer);

        ImGui::NewFrame();

        m::mBool showDemo = true;
        ImGui::ShowDemoWindow(&showDemo);
        ImGui::Render();

        if (m_hdlDx12Surface->isValid)
        {
            m_hdlDx12Surface->surface->render();
        }
        if (m_hdlVulkanSurface->isValid)
        {
            m_hdlVulkanSurface->surface->render();
        }

        return true;
    }

    m::mFloat m_x = 0.0f;
    m::mFloat m_y = 0.0f;

    m::render::IRenderer*           m_iDx12Renderer;
    m::render::IRenderer*           m_iVulkanRenderer;
    m::render::ISurface::HdlPtr     m_hdlDx12Surface;
    m::render::ISurface::HdlPtr     m_hdlVulkanSurface;
    m::input::mCallbackInputManager m_inputManager;
    m::windows::mIWindow*           m_mainDx12Window;
    m::windows::mIWindow*           m_mainVulkanWindow;
    CubeMover                       m_mover;

    const m::logging::mChannelID m_CUBEAPP_ID = mLog_getId();
};

M_EXECUTE_WINDOWED_APP(CubeMoverApp)
