#include <DX12Renderer.hpp>

namespace m
{
namespace dx12
{
DX12Context DX12Context::gs_dx12Contexte;

void DX12Context::init(Bool a_useWarp)
{
    enable_debugLayer();

    m_tearingSupported = check_tearingSupport();

    ComPtr<IDXGIAdapter4> dxgiAdapter4 = get_adapter(a_useWarp);

    m_device = create_device(dxgiAdapter4);

    m_commandQueue.init(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void DX12Context::deinit()
{
    m_commandQueue.destroy();
}

}  // namespace dx12
}  // namespace m