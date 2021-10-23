#ifndef WE_MAIN_H
#define WE_MAIN_H

#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/WindowedApp.hpp>
#include <memory>

using namespace m;

//*****************************************************************************
// App
//*****************************************************************************
class WorldExplorerApp : public m::crossPlatform::IWindowedApplication
{
    void    init() override;
    void    destroy() override;
    m::Bool step(const m::Double& a_deltaTime) override;

    std::unique_ptr<m::render::IRenderer> m_iRenderer;
    m::windows::IWindow*                  m_pWindow;  // gére les inpts
    m::render::ISurface::HdlPtr m_hdlSurface;  // le truc qui sert au rendu

    const m::logging::ChannelID m_WE_LOG_ID = mLOG_GET_ID();

    float               m_fAngle = 0.0f;
    float               m_zPos   = -10;
    input::InputManager m_inputManager;
    void                goDown();
    void                goUp();
};

#endif  // WE_MAIN_H
