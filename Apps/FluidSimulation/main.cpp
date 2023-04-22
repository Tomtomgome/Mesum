#include <MesumCore/Kernel/Kernel.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/DearImgui/MesumDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTaskDearImGui.hpp>
#include <MesumGraphics/ApiAbstraction.hpp>
#include <Kernel/Profile.hpp>

#include "RenderTaskFluidSimulation.hpp"
#include "RendererUtils.hpp"
#include "RenderTasksBasicSwapchain.hpp"
#include <iomanip>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

const m::logging::mChannelID m_FluidSimulation_ID = mLog_getId();

using namespace m;

static const int s_nbRow = 20 * 16;
static const int s_nbCol = 20 * 16;

static const int s_windowZoom = 2;

static const mDouble s_ambientT = 270;

mInt convert_toIndex(mInt a_col, mInt a_row)
{
    return a_row * s_nbCol + a_col;
}

mFloat get_ambientTemperature(mFloat a_altitude)
{
    static const mFloat temperatureGround = 265.0f;
    static const mFloat transitionHeight  = 8000.0f;
    static const mFloat lapseRate         = 0.0065f;

    float temperatureAir;
    if (a_altitude <= transitionHeight)
    {
        temperatureAir = temperatureGround - a_altitude * lapseRate;
    }
    else
    {
        temperatureAir = temperatureGround - transitionHeight * lapseRate +
                         (a_altitude - transitionHeight) * lapseRate;
    }
    return temperatureAir;
}

mFloat get_thermalTemperature(mFloat a_altitude, mFloat a_vaporRatio)
{
    static const mFloat temperatureGround = 265.0f;
    static const mFloat lapseRate         = 0.0065f;

    static const float isentropicAir = 1.4f;//
    static const float isentropicWater = 1.33;

    static const float molarMassAir = 28.96f; // g/mol
    static const float molarMassWater = 18.02f; // g/mol

    float pressure =
        pow((1 - lapseRate * a_altitude / temperatureGround), 5.2561);

    float moleFractionVapor = a_vaporRatio/(a_vaporRatio + 1);
    float molarMassThermal = moleFractionVapor*molarMassWater + (1-moleFractionVapor)*molarMassAir; // g/mol
    float massFractionVapor = moleFractionVapor*molarMassWater/molarMassThermal;
    float isentropicThermal = massFractionVapor*isentropicWater + (1-massFractionVapor)*isentropicAir;

    float temperatureThermal =
        temperatureGround *
        pow((pressure),
            (isentropicThermal - 1.0f / isentropicThermal));

    return temperatureThermal;
}

void init_initialData(m::resource::mTypedImage<m::math::mVec4>& a_image)
{
    for (mInt row = 0; row < s_nbRow; ++row)
    {
        for (mInt col = 0; col < s_nbCol; ++col)
        {
            mFloat altitude       = 0.5 * 30.0f + row * 30.0f;
            mInt   index          = row * s_nbCol + col;
            a_image.data[index].x = 0.0f;                              // qv
            a_image.data[index].y = 0.0f;                              // qc
            a_image.data[index].z = 0.0f;                              // qr
            a_image.data[index].a = get_ambientTemperature(altitude);  // T
        }
    }

    for (mInt row = 0; row < 10; ++row)
    {
        for (mInt col = 2; col < s_nbCol-2; ++col)
        {
            float x = 0.07*(s_nbCol - col - 1);
            float vaporHeight = 5 + 4 * (sin(2 * x) + sin(3.1415 * x));
            if(row < vaporHeight)
            {
                mFloat altitude = 0.5 * 30.0f + row * 30.0f;
                a_image.data[convert_toIndex(col, row)].x = 0.01;
                a_image.data[convert_toIndex(col, row)].a =
                    get_thermalTemperature(altitude, 0.01);  // T
            }
        }
    }
    // a_image.data[convert_toIndex(s_nbCol / 2, 1)].y = 200.0;
    // a_image.data[convert_toIndex(s_nbCol / 2, 1)].a = 350.0;
    /*
    a_image.data[convert_toIndex(s_nbCol / 2 + 1, 1)].z = 350.0;
    a_image.data[convert_toIndex(s_nbCol / 2 + 1, 1)].a = 200.0;
    a_image.data[convert_toIndex(s_nbCol / 2 - 1, 1)].z = 350.0;
    a_image.data[convert_toIndex(s_nbCol / 2 - 1, 1)].a = 200.0;

    a_image.data[convert_toIndex(s_nbCol / 2, 2)].z     = 350.0;
    a_image.data[convert_toIndex(s_nbCol / 2, 2)].a     = 200.0;
    a_image.data[convert_toIndex(s_nbCol / 2 + 1, 2)].z = 350.0;
    a_image.data[convert_toIndex(s_nbCol / 2 + 1, 2)].a = 200.0;
    a_image.data[convert_toIndex(s_nbCol / 2 - 1, 2)].z = 350.0;
    a_image.data[convert_toIndex(s_nbCol / 2 - 1, 2)].a = 200.0;

    a_image.data[convert_toIndex(s_nbCol / 4, 1)].z = 350.0;
    a_image.data[convert_toIndex(s_nbCol / 4, 1)].a = 200.0;

    a_image.data[convert_toIndex(3 * s_nbCol / 4, 1)].z = 350.0;
    a_image.data[convert_toIndex(3 * s_nbCol / 4, 1)].a = 200.0;
    //*/
}

void display_timerTree(TimerTree& a_timerTree)
{
    ImGui::Text("- %s : %.3fms", a_timerTree.name.c_str(),
                a_timerTree.duration);
    ImGui::Indent(15.f);
    for (auto& child : a_timerTree.children)
    {
        display_timerTree(unref_safe(child));
    }
    ImGui::Unindent(15.f);
}

class FluidSimulationApp : public m::crossPlatform::IWindowedApplication
{
    void init(m::mCmdLine const& a_cmdLine, void* a_appData) override
    {
        m::crossPlatform::IWindowedApplication::init(a_cmdLine, a_appData);

        // Simulation setup
        m_initialData.height = s_nbRow;
        m_initialData.width  = s_nbCol;
        m_initialData.data.resize(s_nbCol * s_nbRow);
        init_initialData(m_initialData);

        // Window setup
        m::mCmdLine const& cmdLine          = a_cmdLine;
        m::mUInt           width            = s_windowZoom * s_nbCol;
        m::mUInt           height           = s_windowZoom * s_nbRow;
        m_simulationParameters.screenSize.x = width;
        m_simulationParameters.screenSize.y = height;

        m_pDx12Api = new m::dx12::mApi();
        m_pDx12Api->init();
        auto& rDx12Api = *m_pDx12Api;

        m_mainWindow = add_newWindow("Fluid Simulation", width, height, false);

        m_tasksetExecutor.init();

        static const mUInt           s_nbBackBuffer = 3;
        m::render::mISynchTool::Desc desc{s_nbBackBuffer};

        auto& dx12SynchTool = rDx12Api.create_synchTool();
        m_pDx12SynchTool    = &dx12SynchTool;
        dx12SynchTool.init(desc);

        auto& dx12Swapchain = rDx12Api.create_swapchain();
        m_pDx12Swapchain    = &dx12Swapchain;
        m::render::init_swapchainWithWindow(*m_pDx12Api, m_tasksetExecutor,
                                            dx12Swapchain, dx12SynchTool,
                                            *m_mainWindow, s_nbBackBuffer);

        m::dearImGui::init(*m_mainWindow);

        // Render Taskset setup
        m::render::Taskset& taskset_renderPipeline =
            rDx12Api.create_renderTaskset();

        m::render::mTaskDataSwapchainWaitForRT taskData_swapchainWaitForRT{};
        taskData_swapchainWaitForRT.pSwapchain = m_pDx12Swapchain;
        taskData_swapchainWaitForRT.pSynchTool = m_pDx12SynchTool;
        auto& acquireTask = static_cast<m::render::mTaskSwapchainWaitForRT&>(
            taskData_swapchainWaitForRT.add_toTaskSet(taskset_renderPipeline));

        TaskDataFluidSimulation taskdata_fluidSimulation;
        taskdata_fluidSimulation.pOutputRT    = acquireTask.pOutputRT;
        taskdata_fluidSimulation.pParameters  = &m_simulationParameters;
        taskdata_fluidSimulation.pInitialData = &m_initialData;
        m_pFluidSimulationTask = &static_cast<TaskFluidSimulation&>(
            taskdata_fluidSimulation.add_toTaskSet(taskset_renderPipeline));

        m::render::TaskDataDrawDearImGui taskData_drawDearImGui;
        taskData_drawDearImGui.nbFrames  = s_nbBackBuffer;
        taskData_drawDearImGui.pOutputRT = acquireTask.pOutputRT;
        taskData_drawDearImGui.add_toTaskSet(taskset_renderPipeline);

        m::render::mTaskDataSwapchainPresent taskData_swapchainPresent{};
        taskData_swapchainPresent.pSwapchain = m_pDx12Swapchain;
        taskData_swapchainPresent.pSynchTool = m_pDx12SynchTool;
        taskData_swapchainPresent.add_toTaskSet(taskset_renderPipeline);

        m_tasksetExecutor.confy_permanentTaskset(m::unref_safe(m_pDx12Api),
                                                 taskset_renderPipeline);
        m_mainWindow->attach_toDestroy(m::mCallback<void>(
            [this, &rDx12Api, &taskset_renderPipeline]()
            {
                m_tasksetExecutor.remove_permanentTaskset(
                    rDx12Api, taskset_renderPipeline);
            }));

        m_mainWindow->link_inputManager(&m_inputManager);

        m_inputManager.attach_toKeyEvent(
            m::input::mKeyAction::keyPressed(m::input::keyF11),
            m::input::mKeyActionCallback(
                m_mainWindow, &m::windows::mIWindow::toggle_fullScreen));

        m_inputManager.attach_toKeyEvent(
            m::input::mKeyAction::keyPressed(m::input::keyL),
            m::input::mKeyActionCallback(
                [] { mEnable_logChannels(m_FluidSimulation_ID); }));

        m_mainWindow->attach_toResize(m::windows::mIWindow::mOnResizeCallback(
            [this](mU32 a_width, mU32 a_height)
            {
                this->m_simulationParameters.screenSize.x = a_width;
                this->m_simulationParameters.screenSize.y = a_height;
            }));

        mDisable_logChannels(m_FluidSimulation_ID);

        set_minimalStepDuration(std::chrono::milliseconds(16));
    }

    void destroy() override
    {
        m::crossPlatform::IWindowedApplication::destroy();

        m_pDx12SynchTool->destroy();
        m_pDx12Api->destroy_synchTool(*m_pDx12SynchTool);

        m_pDx12Swapchain->destroy();
        m_pDx12Api->destroy_swapchain(*m_pDx12Swapchain);

        m_pDx12Api->destroy();
        delete m_pDx12Api;

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

        static mBool displayTmp    = false;
        static mBool displaySmoke  = true;
        static mBool displaySpeed  = true;
        static mBool runtimeUpdate = false;

        start_dearImGuiNewFrame(*m_pDx12Api);

        ImGui::NewFrame();

        ImGui::Begin("Simulation Parameters");
        ImGui::Text(
            "FPS: %f",
            1000000.0 / std::chrono::duration_cast<std::chrono::microseconds>(
                            a_deltaTime)
                            .count());
        static mInt minMs = 16;
        ImGui::DragInt("Min Ms", &minMs, 1.0f, 1, 16);
        set_minimalStepDuration(std::chrono::milliseconds(minMs));

        // --- Simulation parameters
        ImGui::Checkbox("Run Time Update", &m_simulationParameters.isRunning);

        if (ImGui::TreeNode("Bounds"))
        {
            ImGui::Checkbox("Left", &m_simulationParameters.leftBoundIsWall);
            ImGui::Checkbox("Right", &m_simulationParameters.rightBoundIsWall);
            ImGui::Checkbox("Top", &m_simulationParameters.topBoundIsWall);
            ImGui::Checkbox("Bottom",
                            &m_simulationParameters.bottomBoundIsWall);

            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Solver"))
        {
            const char* solverNames[static_cast<mInt>(
                TaskDataFluidSimulation::ControlParameters::Solver::_count)] = {
                "Jacobi", "Multi Grid V"};
            const char* solverPreview =
                solverNames[static_cast<mInt>(m_simulationParameters.solver)];

            if (ImGui::BeginCombo("Solver", solverPreview))
            {
                for (mInt i = 0; i < static_cast<mInt>(
                                         TaskDataFluidSimulation::
                                             ControlParameters::Solver::_count);
                     ++i)
                {
                    auto iType = static_cast<
                        TaskDataFluidSimulation::ControlParameters::Solver>(i);
                    if (ImGui::Selectable(
                            solverNames[i],
                            iType == m_simulationParameters.solver))
                    {
                        m_simulationParameters.solver = iType;
                    }
                }
                ImGui::EndCombo();
            }

            if (ImGui::TreeNode("Parameters"))
            {
                switch (m_simulationParameters.solver)
                {
                    default:
                    case TaskDataFluidSimulation::ControlParameters::Solver::
                        jacobi:
                    {
                        ImGui::DragInt(
                            "Jacobi Iterations",
                            &m_simulationParameters.nbJacobiIterations, 1, 1,
                            5000);
                    }
                    break;
                    case TaskDataFluidSimulation::ControlParameters::Solver::
                        multiGridV:
                    {
                        ImGui::DragInt("MG Iterations",
                                       &m_simulationParameters.nbMGIterations,
                                       1, 1, 10);
                        ImGui::DragInt(
                            "MG Jacobi Iterations",
                            &m_simulationParameters.nbMGJacobiIterations, 1, 1,
                            1000);
                        ImGui::DragInt("MG depth",
                                       &m_simulationParameters.maxMGDepth, 1, 1,
                                       6);
                    }
                    break;
                }

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }

        // --- Displays
        ImGui::Checkbox("Displays fluid", &m_simulationParameters.displayFluid);
        ImGui::Checkbox("Displays speeds",
                        &m_simulationParameters.displaySpeed);
        ImGui::Checkbox("Displays simulation debug",
                        &m_simulationParameters.displaySimulationDebug);
        ImGui::DragInt2(
            "Arrows resolution",
            m_simulationParameters.vectorRepresentationResolution.data, 1, 16,
            20 * 16);

        const char* debugDisplayNames[static_cast<mInt>(
            TaskDataFluidSimulation::ControlParameters::DebugDisplays::
                _count)]    = {"None", "Pressure", "Divergence", "Residual"};
        const char* preview = debugDisplayNames[static_cast<mInt>(
            m_simulationParameters.debugDisplay)];

        if (ImGui::BeginCombo("Debug Display", preview))
        {
            for (mInt i = 0;
                 i <
                 static_cast<mInt>(TaskDataFluidSimulation::ControlParameters::
                                       DebugDisplays::_count);
                 ++i)
            {
                auto iType = static_cast<
                    TaskDataFluidSimulation::ControlParameters::DebugDisplays>(
                    i);
                if (ImGui::Selectable(
                        debugDisplayNames[i],
                        iType == m_simulationParameters.debugDisplay))
                {
                    m_simulationParameters.debugDisplay = iType;
                }
            }
            ImGui::EndCombo();
        }

        // --- Timmings
        ImGui::Begin("GPU Timmings");
        display_timerTree(m_pFluidSimulationTask->m_timers[0]);
        ImGui::End();

        ImGui::End();

        ImGui::Render();

        m_tasksetExecutor.run();

        return true;
    }

    m::render::mIApi*       m_pDx12Api;
    m::render::mISwapchain* m_pDx12Swapchain;
    m::render::mISynchTool* m_pDx12SynchTool;

    m::render::mTasksetExecutor m_tasksetExecutor;

    m::input::mCallbackInputManager m_inputManager;
    m::windows::mIWindow*           m_mainWindow;

    m::resource::mTypedImage<m::math::mVec4>   m_initialData;
    TaskDataFluidSimulation::ControlParameters m_simulationParameters;
    TaskFluidSimulation*                       m_pFluidSimulationTask;
};

M_EXECUTE_WINDOWED_APP(FluidSimulationApp)
