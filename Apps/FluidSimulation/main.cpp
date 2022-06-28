#include <MesumCore/Kernel/Kernel.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/DearImgui/MesumDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTaskDearImGui.hpp>
#include <MesumGraphics/ApiAbstraction.hpp>
#include <Kernel/Profile.hpp>

#include "RenderTaskFluidSimulation.hpp"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

using namespace m;

static const mFloat s_cellSize = 1.0f;
static const mFloat s_density  = 1.0f;

static const mInt   s_maxIteration       = 100;
static const mFloat s_solutionTolerance  = 0.0000001;
static const mFloat s_micTunningConstant = 0.97f;

static const mFloat s_gravity = 9.8;
static const mFloat s_wind    = 20;

static const mInt g_gridSize = s_nbRow * s_nbCol;
using GridVector             = math::mVec<mFloat, g_gridSize>;
template <typename t_Type>
using GridVectorSP = math::mVec<t_Type, g_gridSize>;

mBool check_visibility(math::mIVec2 a_target, math::mIVec2 a_pos,
                       mDouble a_viewDistance)
{
    a_target -= a_pos;
    return sqlength(a_target) < a_viewDistance * a_viewDistance;
}

struct EntryOfA
{
    mFloat Adiag;
    mFloat Aplusi;
    mFloat Aplusj;
};

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
            mInt index          = row * s_nbCol + col;
            a_input.Q[index].T  = 0.0f;
            a_input.Q[index].uv = 0.0f;
            a_input.Q[index].uh = 0.0f;

            /*if (row != s_nbRow - 1 && row != 0)
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
            }*/
        }
    }

    a_input.Q[convert_toIndex(s_nbCol / 2, s_nbRow / 2)].T = 100;
}

math::mVec2 get_speedAt(Universe const& a_input, math::mIVec2 a_position)
{
    mFloat tSpeed = (a_position.y == s_nbRow - 1)
                        ? 0.0f
                        : a_input.Q[convert_toIndex(a_position)].uv;
    mFloat rSpeed = (a_position.x == s_nbCol - 1)
                        ? 0.0f
                        : a_input.Q[convert_toIndex(a_position)].uh;
    mFloat lSpeed =
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

void compute_divergence(Universe const& a_input, GridVector& a_outDivergences)
{
    for (mInt j = 0; j < s_nbRow; ++j)
    {
        for (mInt i = 0; i < s_nbCol; ++i)
        {
            mFloat viplus12 =
                (j == s_nbRow - 1) ? 0.0f : a_input.Q[convert_toIndex(i, j)].uv;
            mFloat uiplus12 =
                (i == s_nbCol - 1) ? 0.0f : a_input.Q[convert_toIndex(i, j)].uh;
            mFloat uiminus12 =
                (i == 0) ? 0.0f : a_input.Q[convert_toIndex(i - 1, j)].uh;
            mFloat viminus12 =
                (j == 0) ? 0.0f : a_input.Q[convert_toIndex(i, j - 1)].uv;

            mInt index = convert_toIndex(i, j);

            a_outDivergences[index] = (uiplus12 - uiminus12) / s_cellSize +
                                      (viplus12 - viminus12) / s_cellSize;
        }
    }
}

bool is_outOfBoundI(mInt a_i)
{
    return (a_i == s_nbCol) || (a_i == -1);
}

bool is_outOfBoundJ(mInt a_j)
{
    return (a_j == s_nbRow) || (a_j == -1);
}

bool is_outOfBound(mInt a_i, mInt a_j)
{
    return is_outOfBoundI(a_i) || is_outOfBoundJ(a_j);
}

bool is_solid(Universe const& a_input, mInt a_i, mInt a_j)
{
    return is_outOfBound(a_i, a_j);
}

mInt get_numberOfSolidNeighbors(Universe const& a_input, mInt a_i, mInt a_j)
{
    mInt nbSolid = 0;
    nbSolid += is_solid(a_input, a_i + 1, a_j);
    nbSolid += is_solid(a_input, a_i - 1, a_j);
    nbSolid += is_solid(a_input, a_i, a_j + 1);
    nbSolid += is_solid(a_input, a_i, a_j - 1);
    return nbSolid;
}

void compute_entriesOfA(Universe const& a_input, mFloat a_deltaTime,
                        GridVectorSP<EntryOfA>& a_A)
{
    mFloat globalFactor = a_deltaTime / (s_density * s_cellSize * s_cellSize);
    for (mInt j = 0; j < s_nbRow; ++j)
    {
        for (mInt i = 0; i < s_nbCol; ++i)
        {
            mInt index = convert_toIndex(i, j);
            a_A[index].Adiag =
                globalFactor * (4 - get_numberOfSolidNeighbors(a_input, i, j));
            a_A[index].Aplusi =
                is_solid(a_input, i + 1, j) ? 0.0f : -globalFactor;
            a_A[index].Aplusj =
                is_solid(a_input, i, j + 1) ? 0.0f : -globalFactor;
        }
    }
}

void compute_MIC0Entries(GridVectorSP<EntryOfA> const& a_A, GridVector& MIC0)
{
    for (mInt j = 0; j < s_nbRow; ++j)
    {
        for (mInt i = 0; i < s_nbCol; ++i)
        {
            mInt  index               = convert_toIndex(i, j);
            mBool isOutOfBoundIMinus1 = is_outOfBoundI(i - 1);
            mBool isOutOfBoundJinus1  = is_outOfBoundJ(j - 1);

            mFloat ici;
            mFloat icj;
            mFloat mici;
            mFloat micj;
            if (isOutOfBoundIMinus1)
            {
                ici  = 0.0f;
                mici = 0.0f;
            }
            else
            {
                mInt indexMinusI = convert_toIndex(i - 1, j);
                ici              = a_A[indexMinusI].Aplusi * MIC0[indexMinusI];
                mici = a_A[indexMinusI].Aplusi * (a_A[indexMinusI].Aplusj) *
                       MIC0[indexMinusI] * MIC0[indexMinusI];
            }
            if (isOutOfBoundJinus1)
            {
                icj  = 0.0f;
                micj = 0.0f;
            }
            else
            {
                mInt indexMinusJ = convert_toIndex(i, j - 1);
                icj              = a_A[indexMinusJ].Aplusj * MIC0[indexMinusJ];
                micj = a_A[indexMinusJ].Aplusj * (a_A[indexMinusJ].Aplusi) *
                       MIC0[indexMinusJ] * MIC0[indexMinusJ];
            }

            mFloat e = a_A[index].Adiag - (ici * ici) - (icj * icj) -
                       s_micTunningConstant * (mici + micj);

            mAssert(e >= 0);
            MIC0[index] = 1 / std::sqrt(std::max(e, 0.000000000000000000001f));
        }
    }
}

void apply_preconditionner(GridVector const&             a_vecPreconditionner,
                           GridVectorSP<EntryOfA> const& a_A,
                           GridVector const&             a_vecInput,
                           GridVector&                   a_vecOutput)
{
    for (mInt j = 0; j < s_nbRow; ++j)
    {
        for (mInt i = 0; i < s_nbCol; ++i)
        {
            mInt index       = convert_toIndex(i, j);
            mInt indexMinusI = convert_toIndex(i - 1, j);
            mInt indexMinusJ = convert_toIndex(i, j - 1);

            mFloat fi;
            mFloat fj;
            if (is_outOfBoundI(i - 1))
            {
                fi = 0;
            }
            else
            {
                fi = a_A[indexMinusI].Aplusi *
                     a_vecPreconditionner[indexMinusI] *
                     a_vecOutput[indexMinusI];
            }

            if (is_outOfBoundJ(j - 1))
            {
                fj = 0;
            }
            else
            {
                fj = a_A[indexMinusJ].Aplusj *
                     a_vecPreconditionner[indexMinusJ] *
                     a_vecOutput[indexMinusJ];
            }

            mFloat tmp         = a_vecInput[index] - fi - fj;
            a_vecOutput[index] = tmp * a_vecPreconditionner[index];
        }
    }

    for (mInt j = s_nbRow - 1; j >= 0; --j)
    {
        for (mInt i = s_nbCol - 1; i >= 0; --i)
        {
            mInt index      = convert_toIndex(i, j);
            mInt indexPlusI = convert_toIndex(i + 1, j);
            mInt indexPlusJ = convert_toIndex(i, j + 1);

            mFloat fi;
            mFloat fj;
            if (is_outOfBoundI(i + 1))
            {
                fi = 0;
            }
            else
            {
                fi = a_A[index].Aplusi * a_vecPreconditionner[index] *
                     a_vecOutput[indexPlusI];
            }

            if (is_outOfBoundJ(j + 1))
            {
                fj = 0;
            }
            else
            {
                fj = a_A[index].Aplusj * a_vecPreconditionner[index] *
                     a_vecOutput[indexPlusJ];
            }

            mFloat tmp         = a_vecOutput[index] - fi - fj;
            a_vecOutput[index] = tmp * a_vecPreconditionner[index];
        }
    }
}

void apply_A(GridVectorSP<EntryOfA> const& a_A, GridVector const& a_vecInput,
             GridVector& a_vecOutput)
{
    for (mInt j = 0; j < s_nbRow; ++j)
    {
        for (mInt i = 0; i < s_nbCol; ++i)
        {
            mInt index       = convert_toIndex(i, j);
            mInt indexPlusI  = convert_toIndex(i + 1, j);
            mInt indexMinusI = convert_toIndex(i - 1, j);
            mInt indexPlusJ  = convert_toIndex(i, j + 1);
            mInt indexMinusJ = convert_toIndex(i, j - 1);

            mFloat fPlusI  = 0.0f;
            mFloat fMinusI = 0.0f;
            mFloat fPlusJ  = 0.0f;
            mFloat fMinusJ = 0.0f;
            if (!is_outOfBoundI(i+1))
            {
                fPlusI = a_vecInput[indexPlusI] * a_A[index].Aplusi;
            }
            if (!is_outOfBoundI(i-1))
            {
                fMinusI = a_vecInput[indexMinusI] * a_A[indexMinusI].Aplusi;
            }
            if (!is_outOfBoundJ(j+1))
            {
                fPlusJ = a_vecInput[indexPlusJ] * a_A[index].Aplusj;
            }
            if (!is_outOfBoundJ(j-1))
            {
                fMinusJ = a_vecInput[indexMinusJ] * a_A[indexMinusJ].Aplusj;
            }
            a_vecOutput[index] = a_vecInput[index] * a_A[index].Adiag + fPlusI +
                                 fPlusJ + fMinusI + fMinusJ;
        }
    }
}

mFloat get_maxValue(GridVector& a_vector)
{
    mFloat maxValue = 0;
    for (mInt i = 0; i < g_gridSize; ++i)
    {
        if (a_vector[i] > maxValue)
        {
            maxValue = a_vector[i];
        }
    }
    return maxValue;
}

mBool compute_preconditionedConjugaiteGradient(
    GridVector const& a_vecPreconditionner, GridVectorSP<EntryOfA> const& a_A,
    GridVector& a_pressures, GridVector const& a_divergences,
    mFloat a_fluidDensity)
{
    GridVector vecResidual;
    GridVector vecAuxiliary;
    GridVector vecSearch;
    mFloat     teta;
    for (mInt i = 0; i < s_nbRow * s_nbCol; ++i)
    {
        a_pressures[i] = 0.0f;
        vecResidual[i] = a_divergences[i];
    }
    apply_preconditionner(a_vecPreconditionner, a_A, vecResidual, vecAuxiliary);
    vecSearch = vecAuxiliary;
    // teta      = math::dot(vecAuxiliary, vecResidual);

    for (mInt iterID = 0; iterID < s_maxIteration; ++iterID)
    {
        apply_A(a_A, vecSearch, vecAuxiliary);
        mFloat alpha = a_fluidDensity / dot(vecAuxiliary, vecSearch);
        a_pressures += alpha * vecSearch;
        vecResidual -= alpha * vecAuxiliary;

        if (get_maxValue(vecResidual) <= s_solutionTolerance)
        {
            return true;
        }

        apply_preconditionner(a_vecPreconditionner, a_A, vecResidual,
                              vecAuxiliary);
        teta        = math::dot(vecAuxiliary, vecResidual);
        mFloat beta = teta / a_fluidDensity;
        vecSearch   = vecAuxiliary + beta * vecSearch;
    }

    return false;
}

void update_velocities(GridVector& a_pressures, Universe& a_output,
                       mFloat a_deltaTime)
{
    for (mInt j = 0; j < s_nbRow; ++j)
    {
        for (mInt i = 0; i < s_nbCol; ++i)
        {
            mInt index      = convert_toIndex(i, j);
            mInt indexPlusI = convert_toIndex(i + 1, j);
            mInt indexPlusJ = convert_toIndex(i, j + 1);

            mFloat pressure = a_pressures[index];
            mFloat pressurePlusI;
            mFloat pressurePlusJ;

            if (is_outOfBoundI(i+1))
            {
                pressurePlusI = a_pressures[index] + (s_density * s_cellSize *
                                                      (a_output.Q[index].uh)) /
                                                         a_deltaTime;
            }
            else
            {
                pressurePlusI = a_pressures[indexPlusI];
            }

            if (is_outOfBoundJ(j+1))
            {
                pressurePlusJ = a_pressures[index] + (s_density * s_cellSize *
                                                      (a_output.Q[index].uv)) /
                                                         a_deltaTime;
            }
            else
            {
                pressurePlusJ = a_pressures[indexPlusJ];
            }

            a_output.Q[index].uh = a_output.Q[index].uh -
                                   a_deltaTime * (pressurePlusI - pressure) /
                                       (s_density * s_cellSize);
            a_output.Q[index].uv = a_output.Q[index].uv -
                                   a_deltaTime * (pressurePlusJ - pressure) /
                                       (s_density * s_cellSize);
        }
    }
}

profile::mProfilerMultiSampling<1> g_AdvectionProfiler;
profile::mProfilerMultiSampling<1> g_UpdateProfiler;
profile::mProfilerMultiSampling<1> g_ProjectionProfiler;

void Simulate(Universe const& a_input, Universe& a_output, mFloat a_deltaTime)
{
    mLog_info("SIMULATION STEP ---------------------");
    //---- Advect
    mLog_info("Advect");
    {
        profile::mRAIITiming timming(g_AdvectionProfiler);
        for (mInt row = 0; row < s_nbRow; ++row)
        {
            for (mInt col = 0; col < s_nbCol; ++col)
            {
                mInt        index     = row * s_nbCol + col;
                math::mVec2 cellPoint = {mFloat(col), mFloat(row)};
                // Find particle starting point
                math::mVec2 speed            = get_speedAt(a_input, {col, row});
                math::mVec2 startingPosition = cellPoint - a_deltaTime * speed;

                a_output.Q[index] = get_valueAt(a_input, startingPosition);
            }
        }
    }
    mLog_info("--- Timing : ", g_AdvectionProfiler.get_average<mFloat, std::micro>());

    //---- Apply gravity
    mLog_info("Apply gravity");
    {
        profile::mRAIITiming timming(g_UpdateProfiler);
        for (mInt row = 0; row < s_nbRow; ++row)
        {
            for (mInt col = 0; col < s_nbCol; ++col)
            {
                mInt index = row * s_nbCol + col;
                a_output.Q[index].uh += a_deltaTime * s_gravity;
                a_output.Q[index].uv += a_deltaTime * s_wind;
            }
        }
    }
    mLog_info("--- Timing : ", g_UpdateProfiler.get_average<mFloat, std::micro>());

    //---- Project
    mLog_info("Project");
    {
        profile::mRAIITiming timming(g_ProjectionProfiler);
        // Compute divergences
        mLog_info("- Compute Divergences");
        GridVector divergences;
        compute_divergence(a_output, divergences);
        // Set entries of A
        mLog_info("- Set A");
        GridVectorSP<EntryOfA> A;
        compute_entriesOfA(a_output, a_deltaTime, A);
        // Construct MIC(0)
        mLog_info("- Construct MIC");
        GridVector MIC0;
        compute_MIC0Entries(A, MIC0);
        // Solve Ap = d with MICCG(0)
        mLog_info("- Solve");
        GridVector pressures;
        compute_preconditionedConjugaiteGradient(MIC0, A, pressures, divergences,
                                                 s_density);
        // Change velocities according to pressures
        mLog_info("- Update");
        update_velocities(pressures, a_output, a_deltaTime);
    }
    mLog_info("--- Timing : ", g_ProjectionProfiler.get_average<mFloat, std::micro>());
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

        if(ImGui::Button("Simulation Step"))
        {
            i = (i + 1) % 2;
            Simulate(m_universes[previous], m_universes[i],
                     simulationSpeed * 0.016f);
                         //std::chrono::duration<mFloat>(a_deltaTime).count());
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
