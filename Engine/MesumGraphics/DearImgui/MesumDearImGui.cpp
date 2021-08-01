#include <MesumDearImGui.hpp>
#include <MesumGraphics/Windows.hpp>

namespace m::dearImGui
{
void init(windows::IWindow* a_mainWindow)
{
    mAssert(a_mainWindow != nullptr);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    //        if (a_supportMultiViewports)
    //        {
    //            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    //        }
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();

    a_mainWindow->set_asImGuiWindow();
}

void destroy()
{
    ImGui::DestroyContext();
}

}  // namespace m::dearImGui
