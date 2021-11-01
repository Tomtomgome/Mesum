#include <MesumCore/Kernel/Kernel.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/DearImgui/MesumDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTaskDearImGui.hpp>
#include <MesumGraphics/WindowedApp.hpp>

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
        mLog_to(m_CUBEMOVER_ID, "Se deplace : ", x, ":", y);
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

class CubeMoverApp : public m::crossPlatform::IWindowedApplication
{
    void add_applicationWindow()
    {
        m::windows::IWindow* m_secondWindow =
            add_newWindow("Test Second window", 600, 400);
        m_secondWindow->link_inputManager(&m_inputManager);
    }

    void init() override
    {
        m::crossPlatform::IWindowedApplication::init(<#initializer #>, nullptr);

        m::mCmdLine const& cmdLine = get_cmdLine();
        m::mUInt           width   = 1280;
        m::mUInt           height  = 720;
        if (!cmdLine.get_parameter("-w", width))
        {
            mLog_to(m_CUBEAPP_ID, "Width not overriden, use default : ", width);
        }

        if (!cmdLine.get_parameter("-h", height))
        {
            mLog_to(m_CUBEAPP_ID,
                    "Height not overriden, use default : ", height);
        }

        //m_iRenderer = new m::dx12::DX12Renderer();
        //m_iRenderer = new m::vulkan::VulkanRenderer();
        m_iDx12Renderer = new m::dx12::DX12Renderer();
        m_iDx12Renderer->init();
        m_iVulkanRenderer = new m::vulkan::VulkanRenderer();
        m_iVulkanRenderer->init();

        m_mainDx12Window = add_newWindow("Cube mover app", width, height);
        m_mainDx12Window->set_asMainWindow();

        m_mainVulkanWindow = add_newWindow("Cube mover app", width, height);

        m_hdlDx12Surface = m_mainDx12Window->link_renderer(m_iDx12Renderer);
        m_hdlVulkanSurface = m_mainVulkanWindow->link_renderer(m_iVulkanRenderer);

        m::dearImGui::init(m_mainDx12Window);

        m::render::Taskset* taskset_renderPipeline =
            m_hdlDx12Surface->surface->addNew_renderTaskset();

        m::render::TaskDataDrawDearImGui taskData_drawDearImGui;
        taskData_drawDearImGui.m_hdlOutput = m_hdlDx12Surface;
        taskData_drawDearImGui.add_toTaskSet(taskset_renderPipeline);

        m_mainDx12Window->link_inputManager(&m_inputManager);

        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyPressed(m::input::KEY_F11),
            m::input::KeyActionCallback(
                m_mainDx12Window, &m::windows::IWindow::toggle_fullScreen));

        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyPressed(m::input::KEY_W),
            m::input::KeyActionCallback(this,
                                        &CubeMoverApp::add_applicationWindow));

        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyPressed(m::input::KEY_UP),
            m::input::KeyActionCallback(&m_mover, &CubeMover::set_moveUp));
        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyPressed(m::input::KEY_DOWN),
            m::input::KeyActionCallback(&m_mover, &CubeMover::set_moveDown));
        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyPressed(m::input::KEY_LEFT),
            m::input::KeyActionCallback(&m_mover, &CubeMover::set_moveLeft));
        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyPressed(m::input::KEY_RIGHT),
            m::input::KeyActionCallback(&m_mover, &CubeMover::set_moveRight));

        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyReleased(m::input::KEY_UP),
            m::input::KeyActionCallback(&m_mover, &CubeMover::set_notMoveUp));
        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyReleased(m::input::KEY_DOWN),
            m::input::KeyActionCallback(&m_mover, &CubeMover::set_notMoveDown));
        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyReleased(m::input::KEY_LEFT),
            m::input::KeyActionCallback(&m_mover, &CubeMover::set_notMoveLeft));
        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyReleased(m::input::KEY_RIGHT),
            m::input::KeyActionCallback(&m_mover,
                                        &CubeMover::set_notMoveRight));

        set_microSecondsLimit(16000);
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

    m::mBool step(const m::mDouble& a_deltaTime) override
    {
        if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
        {
            return false;
        }

        m_inputManager.processAndUpdate_States();

        m::mCmdLine const& cmdLine = get_cmdLine();
        if (cmdLine.get_arg("-NoLog"))
        {
            mDisable_logChannels(m_CUBEAPP_ID);
        }

        mLog_to(m_CUBEAPP_ID, "Bonjour !, dt = ", a_deltaTime, "ms");

        m_mover.move(m_x, m_y);

        start_dearImGuiNewFrame(m_iDx12Renderer);

        ImGui::NewFrame();

        m::mBool showDemo = true;
        ImGui::ShowDemoWindow(&showDemo);
        ImGui::Render();

        m_hdlDx12Surface->surface->render();
        if(m_hdlVulkanSurface->isValid)
        {
            m_hdlVulkanSurface->surface->render();
        }

        return true;
    }

    m::mFloat m_x = 0.0f;
    m::mFloat m_y = 0.0f;

    m::render::IRenderer*       m_iDx12Renderer;
    m::render::IRenderer*       m_iVulkanRenderer;
    m::render::ISurface::HdlPtr m_hdlDx12Surface;
    m::render::ISurface::HdlPtr m_hdlVulkanSurface;
    m::input::InputManager      m_inputManager;
    m::windows::IWindow*        m_mainDx12Window;
    m::windows::IWindow*        m_mainVulkanWindow;
    CubeMover                   m_mover;

    const m::logging::mChannelID m_CUBEAPP_ID = mLog_getId();
};

M_EXECUTE_WINDOWED_APP(CubeMoverApp)
