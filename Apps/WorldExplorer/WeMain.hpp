#ifndef WE_MAIN_H
#define WE_MAIN_H

#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/WindowedApp.hpp>
#include <MesumGraphics/RendererUtils.hpp>
#include <memory>

#include "Camera.hpp"
#include "CameraOrbitController.hpp"
#include "Kernel/StateInputManager.hpp"  // < ou "?

using namespace m;

//*****************************************************************************
// App
//*****************************************************************************
class WorldExplorerApp : public m::crossPlatform::IWindowedApplication
{
    void  init(mCmdLine const& a_cmdLine, void* a_appData) override;
    void  destroy() override;
    mBool step(std::chrono::steady_clock::duration const& a_deltaTime) override;

    //std::unique_ptr<m::render::IRenderer> m_iRenderer;

    m::render::mIApi*       m_pApi;
    m::render::mISwapchain* m_pSwapchain;
    m::render::mISynchTool* m_pSynchTool;

    m::render::mTasksetExecutor m_tasksetExecutor;

    m::windows::mIWindow*                 m_pWindow;  // gére les inpts

    const m::logging::mChannelID m_WE_LOG_ID = mLog_getId();

    mFloat                    m_angle = 0.0f;
    input::mStateInputManager m_inputManager;

    render::mCamera              m_camera;
    game::mCameraOrbitController m_cameraOrbitController;
};

#endif  // WE_MAIN_H
