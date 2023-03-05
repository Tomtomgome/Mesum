#include <DX12Context.hpp>

#include "imgui_impl_dx12.h"

namespace m::dx12
{
DX12Context* DX12Context::gs_dx12Contexte;

void DX12Context::init(mBool a_useWarp)
{
#ifdef M_DEBUG
    enable_debugLayer();
#endif  // M_DEBUG
    m_tearingSupported = check_tearingSupport();

    ComPtr<IDXGIAdapter4> dxgiAdapter4 = get_adapter(a_useWarp);
    mDXGIDebugNamed(dxgiAdapter4, "Main context adapter");

    m_device = create_device(dxgiAdapter4);
    mD3D12DebugNamed(m_device, "Main context device");

    m_graphicsCommandQueue.init(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
    m_computeCommandQueue.init(m_device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
}

void DX12Context::deinit()
{
    m_graphicsCommandQueue.destroy();
    m_computeCommandQueue.destroy();
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
}

}  // namespace m::dx12