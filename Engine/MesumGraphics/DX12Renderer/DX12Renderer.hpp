#ifndef M_DX12RENDERER
#define M_DX12RENDERER
#pragma once

#include <MesumCore/Kernel/Asserts.hpp>

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>
//#include <d3dcompiler.h>
#include <DirectXMath.h>

// D3D12 extension library.
#include <d3dx12.h>

#include <algorithm>
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

extern const logging::ChannelID DX_RENDERER_ID;

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
ComPtr<ID3D12GraphicsCommandList> create_commandList(
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

    UInt get_syncInterval() { return m_vSync ? 1 : 0; }

    UInt get_presentFlags()
    {
        return m_tearingSupported && !m_vSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    }

    // DirectX 12 Objects
    ComPtr<ID3D12Device2>      m_device;
    ComPtr<ID3D12CommandQueue> m_commandQueue;

   private:
    // Use WARP adapter
    Bool g_UseWarp = false;

    // Set to true once the DX12 objects have been initialized.
    Bool g_IsInitialized = false;

    // By default, enable V-Sync.
    // Can be toggled with the V key.
    Bool m_vSync            = true;
    Bool m_tearingSupported = false;
};

class DX12Window
{
   public:
    void init(HWND a_hwnd, U32 a_width, U32 a_height);
    void destroy();

    void render();

    void resize(U32 a_width, U32 a_height);

   private:
    void update_renderTargetViews(
        ComPtr<ID3D12Device2> a_device, ComPtr<IDXGISwapChain4> a_swapChain,
        ComPtr<ID3D12DescriptorHeap> a_descriptorHeap);

    // The number of swap chain back buffers.
    static const U8 scm_numFrames = 3;

    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12Resource>  m_backBuffers[scm_numFrames];
    UInt                    m_currentBackBufferIndex;

    ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
    UInt                         m_RTVDescriptorSize;

    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12CommandAllocator>    m_commandAllocators[scm_numFrames];

    // Synchronization objects
    ComPtr<ID3D12Fence> m_fence;
    U64                 m_fenceValue                      = 0;
    U64                 m_frameFenceValues[scm_numFrames] = {};
    HANDLE              m_fenceEvent;

    // Surface description
    U32 m_clientWidth;
    U32 m_clientHeight;
};

// extern DX12Renderer g_dx12Renderer;
}  // namespace dx12
}  // namespace m
#endif //M_DX12RENDERER
