#include "WeMain.hpp"

#include "SrtmHm.hpp"

#include <MesumGraphics/DearImgui/imgui.h>
#include <RenderTasks/RenderTask3dRender.hpp>
#include <MesumGraphics/RenderTasks/RenderTasksBasicSwapchain.hpp>

#include <MesumCore/Kernel/Math.hpp>
#include <MesumDearImGui.hpp>
#include <RenderTasks/RenderTaskDearImGui.hpp>

#include <MesumCore/Kernel/FbxImporter.hpp>
#include "Mesh.hpp"

M_EXECUTE_WINDOWED_APP(WorldExplorerApp)

static m::render::mBasicVertex g_Vertices[8] = {
    {{{-1.0f, -1.0f, -1.0f}}, {{0.0f, 0.0f, 0.0f, 1.0f}}},  // 0
    {{{-1.0f, 1.0f, -1.0f}}, {{0.0f, 1.0f, 0.0f, 1.0f}}},   // 1
    {{{1.0f, 1.0f, -1.0f}}, {{1.0f, 1.0f, 0.0f, 1.0f}}},    // 2
    {{{1.0f, -1.0f, -1.0f}}, {{1.0f, 0.0f, 0.0f, 1.0f}}},   // 3
    {{{-1.0f, -1.0f, 1.0f}}, {{0.0f, 0.0f, 1.0f, 1.0f}}},   // 4
    {{{-1.0f, 1.0f, 1.0f}}, {{0.0f, 1.0f, 1.0f, 1.0f}}},    // 5
    {{{1.0f, 1.0f, 1.0f}}, {{1.0f, 1.0f, 1.0f, 1.0f}}},     // 6
    {{{1.0f, -1.0f, 1.0f}}, {{1.0f, 0.0f, 1.0f, 1.0f}}}     // 7
};

static mU32 g_Indices[36] = {0, 1, 2, 0, 2, 3, 4, 6, 5, 4, 7, 6,
                             4, 5, 1, 4, 1, 0, 3, 2, 6, 3, 6, 7,
                             1, 5, 6, 1, 6, 2, 4, 0, 3, 4, 3, 7};

render::DataMeshBuffer<render::mBasicVertex, mU16> g_meshBuffer;

m::math::mMat4x4 mvpMatrix;

//*****************************************************************************
//
//*****************************************************************************
void WorldExplorerApp::init(mCmdLine const& a_cmdLine, void* a_appData)
{
    m::render::mMesh cube;
    cube.vertices.insert(cube.vertices.end(), &g_Vertices[0], &g_Vertices[8]);
    cube.triangles.insert(cube.triangles.end(), &g_Indices[0], &g_Indices[36]);

    m::crossPlatform::IWindowedApplication::init(a_cmdLine, a_appData);

    /*m::file::mFbxImporter importer;
    importer.PrintFile(
        "C:"
        "\\Perforce\\oProjectSmurfs\\SmurfsResources\\Graph\\Characters\\Smurfs"
        "\\Smurf_Hefty.fbx");*/

    mSrtmHm          srtmHm;
    m::render::mMesh terrain = srtmHm._UpdateMesh();

    mLog_infoTo(m_WE_LOG_ID, "terrain vertices = ", terrain.vertices.size(),
                " | x = ", terrain.vertices[0].position.x,
                " y = ", terrain.vertices[0].position.y,
                " z = ", terrain.vertices[0].position.z);

    mLog_infoTo(
        m_WE_LOG_ID, "terrain vertices = ", terrain.vertices.size(),
        " | x = ", terrain.vertices[terrain.vertices.size() - 1].position.x,
        " y = ", terrain.vertices[terrain.vertices.size() - 1].position.y,
        " z = ", terrain.vertices[terrain.vertices.size() - 1].position.z);

    // m_cameraOrbitController.m_pivot = terrain.m_vertices[0].position;

    m_pApi = new m::dx12::mApi();
    m_pApi->init();

    m_pWindow = add_newWindow("WorldExplorer app", 1280, 720, false);
    m_pWindow->link_inputManager(&m_inputManager);

    m_tasksetExecutor.init();

    static const m::mUInt        nbBackbuffer = 3;
    m::render::mISynchTool::Desc desc{nbBackbuffer};

    auto& synchTool = m_pApi->create_synchTool();
    m_pSynchTool    = &synchTool;
    synchTool.init(desc);

    auto& swapchain = m_pApi->create_swapchain();
    m_pSwapchain    = &swapchain;
    m::render::init_swapchainWithWindow(m::unref_safe(m_pApi),
                                        m_tasksetExecutor, swapchain, synchTool,
                                        m::unref_safe(m_pWindow), nbBackbuffer);

    m::dearImGui::init(*m_pWindow);

    /* ------- Taskset */
    auto& taskset = m_pApi->create_renderTaskset();

    m::render::mTaskDataSwapchainWaitForRT taskData_swapchainWaitForRT{};
    taskData_swapchainWaitForRT.pSwapchain = m_pSwapchain;
    taskData_swapchainWaitForRT.pSynchTool = m_pSynchTool;
    auto& waitTask = static_cast<m::render::mTaskSwapchainWaitForRT&>(
        taskData_swapchainWaitForRT.add_toTaskSet(taskset));

    m::render::mMesh& meshToDraw = terrain;

    // 3dRender task
    g_meshBuffer.m_vertices.insert(g_meshBuffer.m_vertices.begin(),
                                   std::begin(meshToDraw.vertices),
                                   std::end(meshToDraw.vertices));
    g_meshBuffer.m_indices.insert(g_meshBuffer.m_indices.begin(),
                                  std::begin(meshToDraw.triangles),
                                  std::end(meshToDraw.triangles));

    render::TaskData3dRender taskData_3dRender;
    taskData_3dRender.pOutputRT   = waitTask.pOutputRT;
    taskData_3dRender.m_pMeshBuffer = &g_meshBuffer;
    taskData_3dRender.m_matrix      = &mvpMatrix;
    taskData_3dRender.add_toTaskSet(taskset);

    // DearImGui task
    m::render::TaskDataDrawDearImGui taskData_drawDearImGui;
    taskData_drawDearImGui.pOutputRT = waitTask.pOutputRT;
    taskData_drawDearImGui.nbFrames  = nbBackbuffer;
    taskData_drawDearImGui.add_toTaskSet(taskset);

    m::render::mTaskDataSwapchainPresent taskData_swapchainPresent{};
    taskData_swapchainPresent.pSwapchain = m_pSwapchain;
    taskData_swapchainPresent.pSynchTool = m_pSynchTool;
    taskData_swapchainPresent.add_toTaskSet(taskset);

    m_tasksetExecutor.confy_permanentTaskset(m::unref_safe(m_pApi), taskset);
    m_pWindow->attach_toDestroy(m::mCallback<void>(
        [this, &taskset]()
        {
            m_tasksetExecutor.remove_permanentTaskset(m::unref_safe(m_pApi),
                                                      taskset);
        }));
}

//*****************************************************************************
//
//*****************************************************************************
void WorldExplorerApp::destroy()
{
    m::crossPlatform::IWindowedApplication::destroy();

    m_tasksetExecutor.destroy();

    // call to destroy of the swapchain is managed at window termination
    m_pApi->destroy_swapchain(*m_pSwapchain);

    m_pApi->destroy_synchTool(*m_pSynchTool);

    m_pApi->destroy();
    delete m_pApi;

    m::dearImGui::destroy();
}

//*****************************************************************************
//
//*****************************************************************************
mBool WorldExplorerApp::step(
    std::chrono::steady_clock::duration const& a_deltaTime)
{
    if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
    {
        return false;
    }

    mDouble deltaTime = std::chrono::duration<mDouble>(a_deltaTime).count();

    start_dearImGuiNewFrame(m::unref_safe(m_pApi));

    ImGui::NewFrame();
    ImGui::Begin("Engine");
    {
        ImGui::Text("frame time : %f", deltaTime);
        ImGui::Text("frame FPS : %f", 1.0 / deltaTime);
    }
    ImGui::End();
    ImGui::Render();

    m_cameraOrbitController.update(a_deltaTime, m_inputManager);
    m_cameraOrbitController.update_camera(m_camera);

    /* m_angle += m_angle + deltaTime * 90.0f;
    glm::vec3 rotationAxis(0, 1, 0);
    auto      modelMatrix = glm::identity<glm::mat4>();
    glm::rotate(modelMatrix, glm::radians(45.f), rotationAxis);*/

    mFloat aspectRatio = 1280.0f / 720.0f;
    mvpMatrix =
        m_camera.get_viewMatrix() * m_camera.get_projectionMatrix(aspectRatio);

    m_tasksetExecutor.run();

    // mLog_infoTo(m_WE_LOG_ID, "dt = ", deltaTime, "ms");

    // mLog_infoTo(m_WE_LOG_ID, " ", orientation.x, " ", orientation.y, " ",
    // orientation.z);

    return true;
}