#include "WeMain.hpp"

#include <MesumGraphics/DearImgui/imgui.h>
#include <RenderTasks/RenderTask3dRender.h>

#include <MesumCore/Kernel/Math.hpp>
#include <MesumDearImGui.hpp>
#include <RenderTasks/RenderTaskDearImGui.hpp>

M_EXECUTE_WINDOWED_APP(WorldExplorerApp)

static m::render::BasicVertex g_Vertices[8] = {
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

render::DataMeshBuffer<render::BasicVertex, mU16> g_meshBuffer;

using namespace DirectX;
DirectX::XMMATRIX mvpMatrix;

//*****************************************************************************
//
//*****************************************************************************
void WorldExplorerApp::init(mCmdLine const& a_cmdLine, void* a_appData)
{
    m::crossPlatform::IWindowedApplication::init(a_cmdLine, a_appData);

    m_iRenderer = std::make_unique<m::dx12::DX12Renderer>();
    m_iRenderer->init();

    m_pWindow = add_newWindow("WorldExplorer app", 1280, 720);
    m_pWindow->link_inputManager(&m_inputManager);
    m_hdlSurface = m_pWindow->link_renderer(m_iRenderer.get());

    m::dearImGui::init(*m_pWindow);

    /* ------- Taskset */
    render::Taskset* pTaskset = m_hdlSurface->surface->addNew_renderTaskset();

    // 3dRender task
    g_meshBuffer.m_vertices.insert(g_meshBuffer.m_vertices.begin(),
                                   std::begin(g_Vertices),
                                   std::end(g_Vertices));
    g_meshBuffer.m_indices.insert(g_meshBuffer.m_indices.begin(),
                                  std::begin(g_Indices), std::end(g_Indices));

    render::TaskData3dRender taskData_3dRender;
    taskData_3dRender.m_hdlOutput   = m_hdlSurface;
    taskData_3dRender.m_pMeshBuffer = &g_meshBuffer;
    taskData_3dRender.m_matrix      = &mvpMatrix;
    taskData_3dRender.add_toTaskSet(pTaskset);

    // DearImGui task
    render::TaskDataDrawDearImGui taskData_drawDearImGui;
    taskData_drawDearImGui.m_hdlOutput = m_hdlSurface;
    taskData_drawDearImGui.add_toTaskSet(pTaskset);
}

//*****************************************************************************
//
//*****************************************************************************
void WorldExplorerApp::destroy()
{
    m::dearImGui::destroy();

    m_iRenderer->destroy();

    m::crossPlatform::IWindowedApplication::destroy();
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

    start_dearImGuiNewFrame(m_iRenderer.get());

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

    m_angle                     = m_angle + deltaTime * 90.0f;
    const XMVECTOR rotationAxis = XMVectorSet(0, 1, 0, 0);
    XMMATRIX       modelMatrix =
        XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(m_angle));

    mvpMatrix = XMMatrixMultiply(modelMatrix, m_camera.get_viewMatrix());

    mFloat aspectRatio = 1280.0f / 720.0f;
    mvpMatrix =
        XMMatrixMultiply(mvpMatrix, m_camera.get_projectionMatrix(aspectRatio));

    m_hdlSurface->surface->render();

    // mLog_to(m_WE_LOG_ID, "dt = ", deltaTime, "ms");

    // mLog_to(m_WE_LOG_ID, " ", orientation.x, " ", orientation.y, " ",
    // orientation.z);

    return true;
}