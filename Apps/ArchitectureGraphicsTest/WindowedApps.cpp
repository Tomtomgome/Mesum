#include <MesumCore/Kernel/Kernel.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/DearImgui/MesumDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTaskDearImGui.hpp>
#include <MesumGraphics/WindowedApp.hpp>

class CubeMover
{
   public:
    void move(m::Float& x, m::Float& y)
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
        mLOG_TO(m_CUBEMOVER_ID, "Se deplace : ", x, ":", y);
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
    m::Float speed   = 0.016f * 100.f;
    m::Bool  m_up    = false;
    m::Bool  m_down  = false;
    m::Bool  m_left  = false;
    m::Bool  m_right = false;

    const m::logging::ChannelID m_CUBEMOVER_ID = mLOG_GET_ID();
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
        m::crossPlatform::IWindowedApplication::init();

        m::CmdLine const& cmdLine = get_cmdLine();
        m::UInt           width   = 1280;
        m::UInt           height  = 720;
        if (!cmdLine.get_parameter("-w", width))
        {
            mLOG_TO(m_CUBEAPP_ID, "Width not overriden, use default : ", width);
        }

        if (!cmdLine.get_parameter("-h", height))
        {
            mLOG_TO(m_CUBEAPP_ID,
                    "Height not overriden, use default : ", height);
        }

        m_iRenderer = new m::dx12::DX12Renderer();
        m_iRenderer->init();

        m_mainWindow = add_newWindow("Cube mover app", width, height);
        m_mainWindow->set_asMainWindow();

        m_hdlSurface = m_mainWindow->link_renderer(m_iRenderer);

        m::dearImGui::init(m_mainWindow);

        m::render::Taskset* taskset_renderPipeline =
            m_hdlSurface->surface->addNew_renderTaskset();

        m::render::TaskDataDrawDearImGui taskData_drawDearImGui;
        taskData_drawDearImGui.m_hdlOutput = m_hdlSurface;
        taskData_drawDearImGui.add_toTaskSet(taskset_renderPipeline);

        m_mainWindow->link_inputManager(&m_inputManager);

        m_inputManager.attach_ToKeyEvent(
            m::input::KeyAction::keyPressed(m::input::KEY_F11),
            m::input::KeyActionCallback(
                m_mainWindow, &m::windows::IWindow::toggle_fullScreen));

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

        m_iRenderer->destroy();
        delete m_iRenderer;

        m::dearImGui::destroy();
    }

    m::Bool step(const m::Double& a_deltaTime) override
    {
        if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
        {
            return false;
        }

        m_inputManager.processAndUpdate_States();

        m::CmdLine const& cmdLine = get_cmdLine();
        if (cmdLine.get_arg("-NoLog"))
        {
            mLOG_DISABLE(m_CUBEAPP_ID);
        }

        mLOG_TO(m_CUBEAPP_ID, "Bonjour !, dt = ", a_deltaTime, "ms");

        m_mover.move(m_x, m_y);

        start_dearImGuiNewFrame(m_iRenderer);

        ImGui::NewFrame();

        m::Bool showDemo = true;
        ImGui::ShowDemoWindow(&showDemo);
        ImGui::Render();

        m_hdlSurface->surface->render();

        return true;
    }

    m::Float m_x = 0.0f;
    m::Float m_y = 0.0f;

    m::render::IRenderer*       m_iRenderer;
    m::render::ISurface::HdlPtr m_hdlSurface;
    m::input::InputManager      m_inputManager;
    m::windows::IWindow*        m_mainWindow;
    CubeMover                   m_mover;

    const m::logging::ChannelID m_CUBEAPP_ID = mLOG_GET_ID();
};

M_EXECUTE_WINDOWED_APP(CubeMoverApp)
