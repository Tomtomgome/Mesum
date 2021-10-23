#ifndef M_DX12RendererCommon
#define M_DX12RendererCommon
#pragma once

#include <MesumCore/Kernel/Asserts.hpp>
#include <MesumGraphics/Common.hpp>

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>

#include <DirectXMath.h>

// D3D12 extension library.
#include <d3dx12.h>

#include <algorithm>
#include <chrono>
#include <queue>

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

namespace m::dx12
{
// Based on https://www.3dgep.com/learning-directx-12-1/
using namespace Microsoft::WRL;

extern MesumGraphicsApi const logging::ChannelID DX_RENDERER_ID;

inline void check_MicrosoftHRESULT(HRESULT a_hr)
{
    if (FAILED(a_hr))
    {
        mLOG_ERR_TO(DX_RENDERER_ID, "HRESULT FAIL");
        mAssert(false);
    }
}

void set_dxgiDebugName(ComPtr<IDXGIObject> a_dxgiObject, std::string a_sName,
                       const Int a_lineNumber, const Char* a_file);

void set_d3g12DebugName(ComPtr<ID3D12Object> a_d3d12Object, std::string a_sName,
                        const Int a_lineNumber, const Char* a_file);

#ifdef M_DEBUG
#define mDXGIDebugNamed(a_dxgiObject, a_name) \
    set_dxgiDebugName(a_dxgiObject, a_name, __LINE__, __FILE__)
#define mD3D12DebugNamed(a_d3d12Object, a_name) \
    set_d3g12DebugName(a_d3d12Object, a_name, __LINE__, __FILE__)
#else
#define mDXGIDebugNamed(...)
#define mD3D12DebugNamed(...)
#endif

ComPtr<ID3DBlob> compile_shader(std::string const& a_shaderPath,
                                std::string const& a_entryPoint,
                                std::string const& a_target);

void enable_debugLayer();
void report_liveObjects();
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
    uint32_t                    a_numDescriptors,
    D3D12_DESCRIPTOR_HEAP_FLAGS a_flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
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

}  // namespace m::dx12
#endif  // M_DX12RendererCommon
