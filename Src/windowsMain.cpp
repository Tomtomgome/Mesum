#include "PlatWindows/WindowsApp.hpp"


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
    m::WindowsApp app;
    app.setup(hInstance, pCmdLine, nCmdShow);
    app.launch();

    return 0;
}

