#include <DX12Renderer.hpp>
#include "imgui_impl_dx12.h"

namespace m
{
namespace dx12
{

DX12Context* DX12Context::gs_dx12Contexte;

void DX12Context::init(Bool a_useWarp)
{
#ifdef M_DEBUG
    enable_debugLayer();
#endif  // M_DEBUG
    m_tearingSupported = check_tearingSupport();

    ComPtr<IDXGIAdapter4> dxgiAdapter4 = get_adapter(a_useWarp);
    mDXGIDebugNamed(dxgiAdapter4, "Main context adapter");

    m_device = create_device(dxgiAdapter4);
    mD3D12DebugNamed(m_device, "Main context device");

    m_commandQueue.init(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void DX12Context::deinit()
{
    m_commandQueue.destroy();
}

void openRenderModule()
{
    DX12Context::gs_dx12Contexte = new DX12Context();
    DX12Context::gs_dx12Contexte->init();
}

void closeRenderModule()
{
    DX12Context::gs_dx12Contexte->deinit();
    delete DX12Context::gs_dx12Contexte;
#ifdef M_DEBUG
    dx12::report_liveObjects();
#endif  // M_DEBUG
}

void ImGui_RendererNewFrame()
{
    ImGui_ImplDX12_NewFrame();
    DX12Context::gs_dx12Contexte->m_dearImGuiPlatImplCallback.call();
}

}  // namespace dx12
}  // namespace m