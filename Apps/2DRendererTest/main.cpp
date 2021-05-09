#include <File.hpp>
#include <Math.hpp>
#include <MesumGraphics/CrossPlatform.hpp>

using namespace m;

struct RenderNode
{
    // inputs
    // outputs
};

math::RandomGenerator g_randomGenerator;

struct Drawer_2Dprimitives
{
    void init()
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pieplineDesc{};

        dx12::ComPtr<ID3DBlob> vs =
            dx12::compile_shader("data/squareShader.hlsl", "vs_main", "vs_6_0");
        dx12::ComPtr<ID3DBlob> ps =
            dx12::compile_shader("data/squareShader.hlsl", "ps_main", "ps_6_0");


        pieplineDesc.VS.BytecodeLength  = vs->GetBufferSize();
        pieplineDesc.VS.pShaderBytecode = vs->GetBufferPointer();
        pieplineDesc.PS.BytecodeLength  = ps->GetBufferSize();
        pieplineDesc.PS.pShaderBytecode = ps->GetBufferPointer();
    }

    void add_cube(math::Vec2) {}
    void draw() {}

    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;
};

struct BunchOfSquares
{
    void add_newSquare()
    {
        math::Vec2 newPosition;
        newPosition.x = g_randomGenerator.get_nextFloat();
        newPosition.y = g_randomGenerator.get_nextFloat();
        m_squarePositions.push_back(newPosition);
    }

    void update(const Double& a_deltaTime)
    {
        for (auto& position : m_squarePositions)
        {
            position.x += std::sin(Float(a_deltaTime)) * 0.0001f;
            position.y += std::cos(Float(a_deltaTime)) * 0.0001f;
        }
    }

    std::vector<math::Vec2> m_squarePositions;
};

class RendererTestApp : public m::crossPlatform::IWindowedApplication
{
    void init() override
    {
        crossPlatform::IWindowedApplication::init();
        init_renderer(render::RendererApi::DX12);

        g_randomGenerator.init(0);

        m_mainWindow = add_newWindow("Test 2d Renderer", 1280, 720);
        m_mainWindow->link_inputManager(&m_inputManager);
        // link.output(m_mainWindow);
        // 2dPrimitiveManager.output(RenderNode);

        m_inputManager.attach_ToKeyEvent(
            input::KeyAction::keyPressed(input::KEY_N),
            Callback<void>(&m_bunchOfSquares, &BunchOfSquares::add_newSquare));

        m_drawer.init();
    }

    void destroy() override { crossPlatform::IWindowedApplication::destroy(); }

    Bool step(const Double& a_deltaTime) override
    {
        if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
        {
            return false;
        }

        m_bunchOfSquares.update(a_deltaTime);

        for (auto position : m_bunchOfSquares.m_squarePositions)
        {
            m_drawer.add_cube(position);
        }

        m_drawer.draw();

        render();
        return true;
    }

    Drawer_2Dprimitives m_drawer;
    BunchOfSquares      m_bunchOfSquares;
    windows::IWindow*   m_mainWindow = nullptr;
    input::InputManager m_inputManager;
};

M_EXECUTE_WINDOWED_APP(RendererTestApp)