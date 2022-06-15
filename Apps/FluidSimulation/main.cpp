#include <MesumCore/Kernel/Kernel.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/DearImgui/MesumDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTaskDearImGui.hpp>
#include <MesumGraphics/ApiAbstraction.hpp>

#include "RenderTaskFluidSimulation.hpp"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

using namespace m;

mBool check_visibility(math::mIVec2 a_target, math::mIVec2 a_pos,
                       mDouble a_viewDistance)
{
    a_target -= a_pos;
    return sqlength(a_target) < a_viewDistance * a_viewDistance;
}

struct Particle
{
    mFloat uv;
    mFloat uh;
    mFloat T;
};

Particle interpolate(Particle const& a_a, Particle const& a_b,
                     mFloat const a_alpha)
{
    Particle output;
    output.uv = (1.0f - a_alpha) * a_a.uv + a_b.uv * a_alpha;
    output.uh = (1.0f - a_alpha) * a_a.uh + a_b.uh * a_alpha;
    output.T  = (1.0f - a_alpha) * a_a.T + a_b.T * a_alpha;

    return output;
}

mFloat saturate(mFloat const a_input, mFloat const a_min = 0.0f,
                mFloat const a_max = 1.0f)
{
    return std::min(a_max, std::max(a_min, a_input));
}

mInt convert_toIndex(mInt a_x, mInt a_y)
{
    return a_y * s_nbCol + a_x;
}

mInt convert_toIndex(math::mIVec2 a_position)
{
    return convert_toIndex(a_position.x, a_position.y);
}

struct Universe
{
    Particle Q[s_nbRow * s_nbCol];
};

void init_universe(Universe& a_input)
{
    for (mInt row = 0; row < s_nbRow; ++row)
    {
        for (mInt col = 0; col < s_nbCol; ++col)
        {
            mInt index         = row * s_nbCol + col;
            a_input.Q[index].T = 0;

            if (row != s_nbRow - 1 && row != 0)
            {
                a_input.Q[index].uv = 8.0f;
            }
            else
            {
                a_input.Q[index].uv = {};
            }

            if (col != s_nbCol - 1 && col != 0)
            {
                a_input.Q[index].uh = 3.0f;
            }
            else
            {
                a_input.Q[index].uh = {};
            }
        }
    }

    a_input.Q[convert_toIndex(s_nbCol / 2, s_nbRow / 2)].T = 100;
}

math::mVec2 get_speedAt(Universe const& a_input, math::mIVec2 a_position)
{
    mFloat tSpeed = (a_position.y == s_nbRow - 1)
                        ? 0.0f
                        : a_input.Q[convert_toIndex(a_position)].uv;
    mFloat lSpeed = (a_position.x == s_nbCol - 1)
                        ? 0.0f
                        : a_input.Q[convert_toIndex(a_position)].uh;
    mFloat rSpeed =
        (a_position.x == 0)
            ? 0.0f
            : a_input.Q[convert_toIndex(a_position.x - 1, a_position.y)].uh;
    mFloat bSpeed =
        (a_position.y == 0)
            ? 0.0f
            : a_input.Q[convert_toIndex(a_position.x, a_position.y - 1)].uh;

    return {(rSpeed + lSpeed) / 2.0f, (bSpeed + tSpeed) / 2.0f};
}

Particle get_valueAt(Universe const& a_input, math::mVec2 a_position)
{
    math::mIVec2 a_flooredPosition = {mInt(std::floor(a_position.x)),
                                      mInt(std::floor(a_position.y))};
    a_flooredPosition.x =
        std::min(s_nbCol - 1, std::max(0, a_flooredPosition.x));
    a_flooredPosition.y =
        std::min(s_nbRow - 1, std::max(0, a_flooredPosition.y));

    Particle tl =
        a_input
            .Q[convert_toIndex(a_flooredPosition.x, a_flooredPosition.y + 1)];
    Particle tr = a_input.Q[convert_toIndex(a_flooredPosition.x + 1,
                                            a_flooredPosition.y + 1)];
    Particle br =
        a_input
            .Q[convert_toIndex(a_flooredPosition.x + 1, a_flooredPosition.y)];
    Particle bl =
        a_input.Q[convert_toIndex(a_flooredPosition.x, a_flooredPosition.y)];

    Particle top =
        interpolate(tl, tr, saturate(a_position.x - a_flooredPosition.x));
    Particle bottom =
        interpolate(bl, br, saturate(a_position.x - a_flooredPosition.x));

    return interpolate(bottom, top, a_position.y - a_flooredPosition.y);
}

void Simulate(Universe const& a_input, Universe& a_output, mFloat a_deltaTime)
{
    for (mInt row = 0; row < s_nbRow; ++row)
    {
        for (mInt col = 0; col < s_nbCol; ++col)
        {
            mInt        index     = row * s_nbCol + col;
            math::mVec2 cellPoint = {mFloat(col), mFloat(row)};
            //---- Advect
            // Find particle starting point
            math::mVec2 speed            = get_speedAt(a_input, {col, row});
            math::mVec2 startingPosition = cellPoint - a_deltaTime * speed;

            a_output.Q[index] = get_valueAt(a_input, startingPosition);

            //Bound check
            if(row == s_nbRow - 1)
            {
                a_output.Q[index].uv = 0.0f;
            }
            if(col == s_nbCol - 1 || col == 0)
            {
                a_output.Q[index].uh = 0.0f;
            }
            // a_output.Q[index] = a_input.Q[index];
        }
    }
}

class FluidSimulationApp : public m::crossPlatform::IWindowedApplication
{
    void init(m::mCmdLine const& a_cmdLine, void* a_appData) override
    {
        m::crossPlatform::IWindowedApplication::init(a_cmdLine, a_appData);

        m::mCmdLine const& cmdLine = a_cmdLine;
        m::mUInt           width   = 600;
        m::mUInt           height  = 600;

        m_iRenderer = new m::dx12::DX12Renderer();
        m_iRenderer->init();

        m_mainWindow = add_newWindow("Fluid Simulation", width, height);

        m_hdlSurface = m_mainWindow->link_renderer(m_iRenderer);
        m::dearImGui::init(*m_mainWindow);

        m::render::Taskset* taskset_renderPipeline =
            m_hdlSurface->surface->addNew_renderTaskset();

        TaskDataFluidSimulation taskdata_fluidSimulation;
        taskdata_fluidSimulation.m_hdlOutput  = m_hdlSurface;
        taskdata_fluidSimulation.m_pPixelData = &m_pixelData;
        taskdata_fluidSimulation.add_toTaskSet(taskset_renderPipeline);

        m::render::TaskDataDrawDearImGui taskData_drawDearImGui;
        taskData_drawDearImGui.m_hdlOutput = m_hdlSurface;
        taskData_drawDearImGui.add_toTaskSet(taskset_renderPipeline);

        m_mainWindow->link_inputManager(&m_inputManager);

        m_inputManager.attach_toKeyEvent(
            m::input::mKeyAction::keyPressed(m::input::keyF11),
            m::input::mKeyActionCallback(
                m_mainWindow, &m::windows::mIWindow::toggle_fullScreen));

        set_minimalStepDuration(std::chrono::milliseconds(16));

        m_pixelData.resize(s_nbRow * s_nbCol);
        init_universe(m_universes[0]);
    }

    void destroy() override
    {
        m::crossPlatform::IWindowedApplication::destroy();

        m_iRenderer->destroy();
        delete m_iRenderer;

        m::dearImGui::destroy();
    }

    m::mBool step(
        std::chrono::steady_clock::duration const& a_deltaTime) override
    {
        if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
        {
            return false;
        }

        static mFloat simulationSpeed = 1.0f;
        static mInt   i               = 0;
        mInt          previous        = i;

        static mBool displayTmp   = true;
        static mBool displaySpeed = true;

        for (mInt pix = 0; pix < s_nbRow * s_nbCol; ++pix)
        {
            m_pixelData[pix].r =
                displayTmp ? m_universes[previous].Q[pix].T / 10 : 0;

            m_pixelData[pix].g =
                displaySpeed ? m_universes[previous].Q[pix].uh : 0;
            m_pixelData[pix].b =
                displaySpeed ? m_universes[previous].Q[pix].uv : 0;
        }

        i = (i + 1) % 2;
        Simulate(m_universes[previous], m_universes[i],
                 simulationSpeed *
                     std::chrono::duration<mFloat>(a_deltaTime).count());

        start_dearImGuiNewFrame(m_iRenderer);

        ImGui::NewFrame();

        m::mBool showDemo = true;
        ImGui::Begin("Simulation Parameters");
        ImGui::DragFloat("SimulationSpeed", &simulationSpeed, 0.01f, 0.01f,
                         2.0f);
        if (ImGui::Button("Reset"))
        {
            init_universe(m_universes[i]);
        }

        ImGui::Checkbox("Display Tmp", &displayTmp);
        ImGui::Checkbox("Display Speed", &displaySpeed);
        ImGui::End();

        ImGui::Render();

        if (m_hdlSurface->isValid)
        {
            m_hdlSurface->surface->render();
        }

        return true;
    }

    m::render::IRenderer*           m_iRenderer;
    m::render::ISurface::HdlPtr     m_hdlSurface;
    m::input::mCallbackInputManager m_inputManager;
    m::windows::mIWindow*           m_mainWindow;

    std::vector<m::math::mVec4> m_pixelData;
    Universe                    m_universes[2];

    const m::logging::mChannelID m_FluidSimulation_ID = mLog_getId();
};

M_EXECUTE_WINDOWED_APP(FluidSimulationApp)
