#include "WeMain.h"

#include <MesumGraphics/DearImgui/imgui.h>
#include <MesumCore/Kernel/Math.hpp>
#include <MesumDearImGui.hpp>
#include <RenderTasks/RenderTaskDearImGui.hpp>
#include <RenderTasks/RenderTask3dRender.h>

M_EXECUTE_WINDOWED_APP(WorldExplorerApp)

static m::render::BasicVertex g_Vertices[8] = {
		{ { { -1.0f, -1.0f, -1.0f } }, { { 0.0f, 0.0f, 0.0f, 1.0f } } },  // 0
		{ { { -1.0f, 1.0f, -1.0f } }, { { 0.0f, 1.0f, 0.0f, 1.0f } } },   // 1
		{ { { 1.0f, 1.0f, -1.0f } }, { { 1.0f, 1.0f, 0.0f, 1.0f } } },    // 2
		{ { { 1.0f, -1.0f, -1.0f } }, { { 1.0f, 0.0f, 0.0f, 1.0f } } },   // 3
		{ { { -1.0f, -1.0f, 1.0f } }, { { 0.0f, 0.0f, 1.0f, 1.0f } } },   // 4
		{ { { -1.0f, 1.0f, 1.0f } }, { { 0.0f, 1.0f, 1.0f, 1.0f } } },    // 5
		{ { { 1.0f, 1.0f, 1.0f } }, { { 1.0f, 1.0f, 1.0f, 1.0f } } },     // 6
		{ { { 1.0f, -1.0f, 1.0f } }, { { 1.0f, 0.0f, 1.0f, 1.0f } } }     // 7
};

static U32 g_Indices[36] = {
		0, 1, 2, 0,	2, 3,
		4, 6, 5, 4, 7, 6,
		4, 5, 1, 4,	1, 0,
		3, 2, 6, 3, 6, 7,
		1, 5, 6, 1,	6, 2,
		4, 0, 3, 4, 3, 7 };

render::DataMeshBuffer<render::BasicVertex, U16> g_meshBuffer;

using namespace DirectX;
DirectX::XMMATRIX mvpMatrix;

//*****************************************************************************
//
//*****************************************************************************
void WorldExplorerApp::init()
{
	m::crossPlatform::IWindowedApplication::init();

	m_iRenderer = std::make_unique<m::dx12::DX12Renderer>();
	m_iRenderer->init();

	m_pWindow = add_newWindow("WorldExplorer app", 1280, 720);
	m_pWindow->link_inputManager(&m_inputManager);
	m_hdlSurface = m_pWindow->link_renderer(m_iRenderer.get());
	m_pWindow->set_asMainWindow();

	m::dearImGui::init(m_pWindow);

	/* ------- Taskset */
	render::Taskset* pTaskset = m_hdlSurface->surface->addNew_renderTaskset();

	// 3dRender task
	g_meshBuffer.m_vertices.insert(
			g_meshBuffer.m_vertices.begin(),
			std::begin(g_Vertices),
			std::end(g_Vertices));
	g_meshBuffer.m_indices.insert(
			g_meshBuffer.m_indices.begin(),
			std::begin(g_Indices),
			std::end(g_Indices));

	render::TaskData3dRender taskData_3dRender;
	taskData_3dRender.m_hdlOutput   = m_hdlSurface;
	taskData_3dRender.m_pMeshBuffer = &g_meshBuffer;
	taskData_3dRender.m_matrix = &mvpMatrix;
	taskData_3dRender.add_toTaskSet(pTaskset);

	// DearImGui task
	render::TaskDataDrawDearImGui taskData_drawDearImGui;
	taskData_drawDearImGui.m_hdlOutput = m_hdlSurface;
	taskData_drawDearImGui.add_toTaskSet(pTaskset);

	m_inputManager.attach_ToKeyEvent(
			input::KeyAction::keyPressed(input::KEY_DOWN),
			Callback<void>(this, &WorldExplorerApp::goDown));
	m_inputManager.attach_ToKeyEvent(
			input::KeyAction::keyPressed(input::KEY_UP),
			Callback<void>(this, &WorldExplorerApp::goUp));
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
m::Bool WorldExplorerApp::step(const m::Double& a_deltaTime)
{
	if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
	{
		return false;
	}

	start_dearImGuiNewFrame(m_iRenderer.get());

	ImGui::NewFrame();
	ImGui::Begin("Engine");
	{
		ImGui::Text("frame time : %f", a_deltaTime);
		ImGui::Text("frame FPS : %f", 1.0 / a_deltaTime);
	}
	ImGui::End();
	ImGui::Render();


	m_fAngle = m_fAngle + a_deltaTime * 90.0f;
	const XMVECTOR rotationAxis = XMVectorSet( 0, 1, 0, 0 );
	XMMATRIX       modelMatrix  = XMMatrixRotationAxis( rotationAxis, XMConvertToRadians(m_fAngle) );

	// Update the view matrix.
	const XMVECTOR eyePosition = XMVectorSet( -5, 10, m_zPos, 1 );
	const XMVECTOR focusPoint  = XMVectorSet( 0, 0, 0, 1 );
	const XMVECTOR upDirection = XMVectorSet( 0, 1, 0, 0 );
	XMMATRIX       viewMatrix  = XMMatrixLookAtLH( eyePosition, focusPoint, upDirection );

	// Update the projection matrix.
	float aspectRatio = 1280.0f / 720.0f;
	float fieldOfView = 45.0f;
	XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH( XMConvertToRadians( fieldOfView ), aspectRatio, 0.1f, 100.0f );
	mvpMatrix = XMMatrixMultiply( modelMatrix, viewMatrix );
	mvpMatrix = XMMatrixMultiply( mvpMatrix, projectionMatrix );



	m_hdlSurface->surface->render();

	mLOG_TO(m_WE_LOG_ID, "dt = ", a_deltaTime, "ms");

	return true;
}

//*****************************************************************************
//
//*****************************************************************************
void WorldExplorerApp::goDown()
{
	m_zPos -= 1;
}

//*****************************************************************************
//
//*****************************************************************************
void WorldExplorerApp::goUp()
{
	m_zPos += 1;
}