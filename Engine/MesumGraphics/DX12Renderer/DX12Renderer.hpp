#ifndef M_DX12RENDERER
#define M_DX12RENDERER
#pragma once

#include <DX12CommandQueue.hpp>


namespace m
{
namespace dx12
{

class DX12Context
{
   public:
    static DX12Context* gs_dx12Contexte;

    void init(Bool a_useWarp = false);
    void deinit();

    Bool get_tearingSupport() { return m_tearingSupported; }

    DX12CommandQueue& get_commandQueue() { return m_commandQueue; }

    // DirectX 12 Objects
    ComPtr<ID3D12Device2>      m_device;
    DX12CommandQueue           m_commandQueue;


   private:
    // Use WARP adapter
    Bool g_UseWarp = false;

    // Set to true once the DX12 objects have been initialized.
    Bool g_IsInitialized = false;

    Bool m_tearingSupported = false;
};

}  // namespace dx12
}  // namespace m
#endif //M_DX12RENDERER
