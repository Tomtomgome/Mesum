#ifndef M_DX12RENDERER
#define M_DX12RENDERER
#pragma once

#include <Application/Main.hpp>

// DirectX 12 specific headers.
#include <d3d12.h>
#include <dxgi1_6.h>
//#include <d3dcompiler.h>
#include <DirectXMath.h>

// D3D12 extension library.
#include <d3dx12.h>

#include <algorithm>

namespace m
{	
	namespace dx12
	{
		using namespace Microsoft::WRL;

		class DX12Renderer
		{
			// The number of swap chain back buffers.
			static const U8 g_NumFrames = 3;

			// Use WARP adapter
			mBool g_UseWarp = false;

			// Set to true once the DX12 objects have been initialized.
			mBool g_IsInitialized = false;

			// DirectX 12 Objects
			ComPtr<ID3D12Device2> g_Device;
			ComPtr<ID3D12CommandQueue> g_CommandQueue;
			ComPtr<IDXGISwapChain4> g_SwapChain;
			ComPtr<ID3D12Resource> g_BackBuffers[g_NumFrames];
			ComPtr<ID3D12GraphicsCommandList> g_CommandList;
			ComPtr<ID3D12CommandAllocator> g_CommandAllocators[g_NumFrames];
			ComPtr<ID3D12DescriptorHeap> g_RTVDescriptorHeap;
			UInt g_RTVDescriptorSize;
			UInt g_CurrentBackBufferIndex;

			// Synchronization objects
			ComPtr<ID3D12Fence> g_Fence;
			U64 g_FenceValue = 0;
			U64 g_FrameFenceValues[g_NumFrames] = {};
			HANDLE g_FenceEvent;

			// By default, enable V-Sync.
			// Can be toggled with the V key.
			mBool g_VSync = true;
			mBool g_TearingSupported = false;
		};
	}
}
#endif //M_DX12RENDERER
