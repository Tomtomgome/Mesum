#ifndef M_MESUMDEARIMGUI
#define M_MESUMDEARIMGUI
#pragma once

#include <imgui.h>

namespace m::windows
{
class IWindow;
}

namespace m::dearImGui
{
void init(windows::IWindow* a_mainWindow);
void destroy();

}  // namespace m::dearImGui

#endif  // M_MESUMDEARIMGUI