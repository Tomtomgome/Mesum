#include <Kernel/Types.hpp>
#include <Application/Main.hpp>
#include <Logger/Logger.hpp>
#include <Input/Keys.hpp>
#include <Input/Input.hpp>
#include <Input/InputCommon.hpp>
#include <CrossPlatform/CrossPlatform.hpp>


const logging::ChannelID CUBEAPP_ID = LOG_GET_ID();

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
		LOG_TO(CUBEAPP_ID, "Se deplace : ", x, ":", y);
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
	m::Float speed = 0.016 * 100;
	m::mBool  m_up = false;
	m::mBool  m_down = false;
	m::mBool  m_left = false;
	m::mBool  m_right = false;
};

class CubeMoverApp : public m::platform::PlatformApp
{
	virtual void configure() override
	{
		m::platform::PlatformApp::configure();
		m::UInt width = 1280;
		m::UInt height = 720;
		if (!get_cmdLine().get_parameter(L"-w", width))
		{
			LOG_TO(CUBEAPP_ID, "Width not overriden, use default : ", width);
		}

		if (!get_cmdLine().get_parameter(L"-h", height))
		{
			LOG_TO(CUBEAPP_ID, "Height not overriden, use default : ", height);
		}

		set_size(width, height);
		set_windowName(L"Cube mover app");
		link_inputManager(&m_inputManager);
	}

    virtual void init() override
	{
		m::platform::PlatformApp::init();
		m_inputManager.attachToKeyEvent(m::input::KeyAction::keyPressed(m::input::KEY_UP), m::input::KeyActionCallback(&m_mover, &CubeMover::set_moveUp));
		m_inputManager.attachToKeyEvent(m::input::KeyAction::keyPressed(m::input::KEY_DOWN), m::input::KeyActionCallback(&m_mover, &CubeMover::set_moveDown));
		m_inputManager.attachToKeyEvent(m::input::KeyAction::keyPressed(m::input::KEY_LEFT), m::input::KeyActionCallback(&m_mover, &CubeMover::set_moveLeft));
		m_inputManager.attachToKeyEvent(m::input::KeyAction::keyPressed(m::input::KEY_RIGHT), m::input::KeyActionCallback(&m_mover, &CubeMover::set_moveRight));

		m_inputManager.attachToKeyEvent(m::input::KeyAction::keyReleased(m::input::KEY_UP), m::input::KeyActionCallback(&m_mover, &CubeMover::set_notMoveUp));
		m_inputManager.attachToKeyEvent(m::input::KeyAction::keyReleased(m::input::KEY_DOWN), m::input::KeyActionCallback(&m_mover, &CubeMover::set_notMoveDown));
		m_inputManager.attachToKeyEvent(m::input::KeyAction::keyReleased(m::input::KEY_LEFT), m::input::KeyActionCallback(&m_mover, &CubeMover::set_notMoveLeft));
		m_inputManager.attachToKeyEvent(m::input::KeyAction::keyReleased(m::input::KEY_RIGHT), m::input::KeyActionCallback(&m_mover, &CubeMover::set_notMoveRight));

        set_microSecondsLimit(16000);
    }

    virtual void destroy() override
	{
		m::platform::PlatformApp::destroy();
		//Nothing to destroy
    }

    virtual m::mBool step(const m::Double& a_deltaTime) override
    {
		m::mBool signalKeepRunning = m::platform::PlatformApp::step(a_deltaTime);
		if (get_cmdLine().get_arg(L"-NoLog"))
		{
			//LOG_DISABLE(CUBEAPP_ID);
		}

        LOG_TO(CUBEAPP_ID, "Bonjour !, dt = ", a_deltaTime, "ms");

		m_mover.move(m_x, m_y);

        return signalKeepRunning;
    }

	m::Float m_x = 0.0f;
	m::Float m_y = 0.0f;

	m::input::InputManager m_inputManager;
	CubeMover           m_mover;
};

M_EXECUTE_CONSOLE_APP(CubeMoverApp)
