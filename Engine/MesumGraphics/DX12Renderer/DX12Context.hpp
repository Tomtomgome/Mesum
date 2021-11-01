#ifndef M_DX12Context
#define M_DX12Context
#pragma once

#include <DX12CommandQueue.hpp>
#include <MesumCore/Kernel/Callbacks.hpp>

namespace m::dx12
{
class DX12Context
{
   public:
    DX12Context()                    = default;
    DX12Context(DX12Context const&)  = delete;
    DX12Context(DX12Context const&&) = delete;
    ~DX12Context()                   = default;
    static DX12Context* gs_dx12Contexte;

    void init(mBool a_useWarp = false);
    void deinit();

    mBool get_tearingSupport() const { return m_tearingSupported; }

    DX12CommandQueue& get_commandQueue() { return m_commandQueue; }

    // DirectX 12 Objects
    ComPtr<ID3D12Device2> m_device;
    DX12CommandQueue      m_commandQueue;

   private:
    // Use WARP adapter
    mBool g_UseWarp = false;

    // Set to true once the DX12 objects have been initialized.
    mBool g_IsInitialized = false;

    mBool m_tearingSupported = false;
};

}  // namespace m::dx12
#endif  // M_DX12Context
