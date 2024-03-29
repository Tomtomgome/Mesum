#pragma once

#include "imgui.h"

namespace m::windows
{
class mIWindow;
}

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Graphics
/// \{
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// \brief Namespace Mesum specific Dear Im Gui functionalities
///////////////////////////////////////////////////////////////////////////////
namespace m::dearImGui
{
///////////////////////////////////////////////////////////////////////////////
/// \brief Initialize DearImGui on a specified window
///
/// For now dear im gui is only supported on one window.
/// Should be called once, before any other DearImGui related operations
///
/// \param a_mainWindow The windows on witch to initialize Dear Im Gui
///////////////////////////////////////////////////////////////////////////////
void init(windows::mIWindow& a_mainWindow);

///////////////////////////////////////////////////////////////////////////////
/// \brief Destroy DearImGui context
///
/// Should be called once after any other DearImGui related operations
///////////////////////////////////////////////////////////////////////////////
void destroy();

}  // namespace m::dearImGui
///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////