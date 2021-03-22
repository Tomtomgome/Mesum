#ifndef M_DX12RENDERER
#define M_DX12RENDERER
#pragma once

#include <MesumCore/Kernel/Asserts.hpp>
#include <MesumGraphics/Common.hpp>

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>
//#include <d3dcompiler.h>
#include <DirectXMath.h>

// D3D12 extension library.
#include <d3dx12.h>

#include <algorithm>
#include <queue>
#include <chrono>


#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif



namespace m
{
namespace dx12
{
// Based on https://www.3dgep.com/learning-directx-12-1/
using namespace Microsoft::WRL;

extern MesumGraphicsApi const logging::ChannelID DX_RENDERER_ID;

inline void check_MicrosoftHRESULT(HRESULT a_hr)
{
    if (FAILED(a_hr))
    {
        mLOG_ERR_TO(DX_RENDERER_ID, "HRESULT FAIL");
        mHardAssert(false);
    }
}

void enable_debugLayer();
bool check_tearingSupport();

ComPtr<IDXGIAdapter4> get_adapter(Bool a_useWarp);

ComPtr<ID3D12Device2>      create_device(ComPtr<IDXGIAdapter4> a_adapter);
ComPtr<ID3D12CommandQueue> create_commandQueue(ComPtr<ID3D12Device2>   a_device,
                                               D3D12_COMMAND_LIST_TYPE a_type);
ComPtr<IDXGISwapChain4>    create_swapChain(
       HWND a_hWnd, ComPtr<ID3D12CommandQueue> a_commandQueue, uint32_t a_width,
       uint32_t a_height, uint32_t a_bufferCount);
ComPtr<ID3D12DescriptorHeap> create_descriptorHeap(
    ComPtr<ID3D12Device2> a_device, D3D12_DESCRIPTOR_HEAP_TYPE a_type,
    uint32_t a_numDescriptors);
ComPtr<ID3D12CommandAllocator> create_commandAllocator(
    ComPtr<ID3D12Device2> a_device, D3D12_COMMAND_LIST_TYPE a_type);
ComPtr<ID3D12GraphicsCommandList2> create_commandList(
    ComPtr<ID3D12Device2>          a_device,
    ComPtr<ID3D12CommandAllocator> a_commandAllocator,
    D3D12_COMMAND_LIST_TYPE        a_type);
ComPtr<ID3D12Fence> create_fence(ComPtr<ID3D12Device2> a_device);
HANDLE              create_eventHandle();
U64                 signal_fence(ComPtr<ID3D12CommandQueue> a_commandQueue,
                                 ComPtr<ID3D12Fence> a_fence, U64& a_fenceValue);
void                wait_fenceValue(
                   ComPtr<ID3D12Fence> a_fence, uint64_t a_fenceValue, HANDLE a_fenceEvent,
                   std::chrono::milliseconds a_duration = std::chrono::milliseconds::max());
void flush(ComPtr<ID3D12CommandQueue> a_commandQueue,
           ComPtr<ID3D12Fence> a_fence, uint64_t& a_fenceValue,
           HANDLE a_fenceEvent);

class DX12Context
{
   public:
    static DX12Context gs_dx12Contexte;

    void init(Bool a_useWarp = false);
    void deinit();

    Bool get_tearingSupport() { return m_tearingSupported; }

    // DirectX 12 Objects
    ComPtr<ID3D12Device2>      m_device;
    ComPtr<ID3D12CommandQueue> m_commandQueue;

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