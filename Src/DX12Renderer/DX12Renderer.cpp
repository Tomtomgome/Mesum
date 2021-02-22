#include <DX12Renderer.hpp>

namespace m
{
	namespace dx12
	{
		extern const logging::ChannelID DX_RENDERER_ID = LOG_GET_ID();
		//extern DX12Renderer g_dx12Renderer;
		DX12Renderer DX12Renderer::gs_dx12Renderer;

		void enable_debugLayer()
		{
			ComPtr<ID3D12Debug> debugInterface;
			check_MicrosoftHRESULT(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
			debugInterface->EnableDebugLayer();
		}

		mBool check_tearingSupport()
		{
			mBool allowTearing = false;

			// Rather than create the DXGI 1.5 factory interface directly, we create the
			// DXGI 1.4 interface and query for the 1.5 interface. This is to enable the
			// graphics debugging tools which will not support the 1.5 factory interface
			// until a future update.
			ComPtr<IDXGIFactory4> factory4;
			if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
			{
				ComPtr<IDXGIFactory5> factory5;
				if (SUCCEEDED(factory4.As(&factory5)))
				{
					if (FAILED(factory5->CheckFeatureSupport(
						DXGI_FEATURE_PRESENT_ALLOW_TEARING,
						&allowTearing, sizeof(allowTearing))))
					{
						allowTearing = false;
					}
				}
			}

			return allowTearing == true;
		}

		ComPtr<IDXGIAdapter4> get_adapter(mBool a_useWarp)
		{
			ComPtr<IDXGIFactory4> dxgiFactory;
			UInt createFactoryFlags = 0;
#ifdef M_DEBUG
			createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif //M_DEBUG

			check_MicrosoftHRESULT(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

			ComPtr<IDXGIAdapter1> dxgiAdapter1;
			ComPtr<IDXGIAdapter4> dxgiAdapter4;

			if (a_useWarp)
			{
				check_MicrosoftHRESULT(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
				check_MicrosoftHRESULT(dxgiAdapter1.As(&dxgiAdapter4));
			}
			else
			{
				UInt size_maxDedicatedVideoMemory = 0;
				for (UInt i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
				{
					DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
					dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

					if((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0
						&& SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))
						&& dxgiAdapterDesc1.DedicatedVideoMemory > size_maxDedicatedVideoMemory)
					{
						size_maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
						check_MicrosoftHRESULT(dxgiAdapter1.As(&dxgiAdapter4));
					}
				}
			}
			return dxgiAdapter4;
		}

		ComPtr<ID3D12Device2> create_device(ComPtr<IDXGIAdapter4> a_adapter)
		{
			ComPtr<ID3D12Device2> d3d12Device2;
			check_MicrosoftHRESULT(D3D12CreateDevice(a_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device2)));

#ifdef M_DEBUG
			ComPtr<ID3D12InfoQueue> infoQueue;
			if (SUCCEEDED(d3d12Device2.As(&infoQueue)))
			{
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
				// Suppress whole categories of messages
				//D3D12_MESSAGE_CATEGORY list_categories[] = {};

				// Suppress messages based on their severity level
				D3D12_MESSAGE_SEVERITY list_severities[] =
				{
					D3D12_MESSAGE_SEVERITY_INFO
				};

				// Suppress individual messages by their ID
				D3D12_MESSAGE_ID list_denyIds[] = {
					D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
					D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
					D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
				};

				D3D12_INFO_QUEUE_FILTER filter = {};
				//filter.DenyList.NumCategories = _countof(list_categories);
				//filter.DenyList.pCategoryList = list_categories;
				filter.DenyList.NumSeverities = _countof(list_severities);
				filter.DenyList.pSeverityList = list_severities;
				//filter.DenyList.NumIDs = _countof(list_denyIds);
				//filter.DenyList.pIDList = list_denyIds;

				check_MicrosoftHRESULT(infoQueue->PushStorageFilter(&filter));
			}
#endif
			return d3d12Device2;
		}

		ComPtr<ID3D12CommandQueue> create_commandQueue(ComPtr<ID3D12Device2> a_device, D3D12_COMMAND_LIST_TYPE a_type)
		{
			ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

			D3D12_COMMAND_QUEUE_DESC desc_commandQueue = {};
			desc_commandQueue.Type		= a_type;
			desc_commandQueue.Priority	= D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
			desc_commandQueue.Flags		= D3D12_COMMAND_QUEUE_FLAG_NONE;
			desc_commandQueue.NodeMask	= 0;

			check_MicrosoftHRESULT(a_device->CreateCommandQueue(&desc_commandQueue, IID_PPV_ARGS(&d3d12CommandQueue)));

			return d3d12CommandQueue;
		}

		ComPtr<IDXGISwapChain4> create_swapChain(HWND a_hWnd, ComPtr<ID3D12CommandQueue> a_commandQueue, uint32_t a_width, uint32_t a_height, uint32_t a_bufferCount)
		{
			ComPtr<IDXGISwapChain4> dxgiSwapChain4;
			ComPtr<IDXGIFactory4> dxgiFactory4;
			UINT flags_createFactory = 0;
#ifdef M_DEBUG
			flags_createFactory = DXGI_CREATE_FACTORY_DEBUG;
#endif //M_DEBUG

			check_MicrosoftHRESULT(CreateDXGIFactory2(flags_createFactory, IID_PPV_ARGS(&dxgiFactory4)));

			DXGI_SWAP_CHAIN_DESC1 desc_SwapChain = {};
			desc_SwapChain.Width = a_width;
			desc_SwapChain.Height = a_height;
			desc_SwapChain.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc_SwapChain.Stereo = false;
			desc_SwapChain.SampleDesc = { 1, 0 };
			desc_SwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			desc_SwapChain.BufferCount = a_bufferCount;
			desc_SwapChain.Scaling = DXGI_SCALING_STRETCH;
			desc_SwapChain.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			desc_SwapChain.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
			desc_SwapChain.Flags = check_tearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

			ComPtr<IDXGISwapChain1> swapChain1;
			check_MicrosoftHRESULT(dxgiFactory4->CreateSwapChainForHwnd(
				a_commandQueue.Get(),
				a_hWnd,
				&desc_SwapChain,
				nullptr,
				nullptr,
				&swapChain1));

			// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
			// will be handled manually.
			check_MicrosoftHRESULT(dxgiFactory4->MakeWindowAssociation(a_hWnd, DXGI_MWA_NO_ALT_ENTER));

			check_MicrosoftHRESULT(swapChain1.As(&dxgiSwapChain4));

			return dxgiSwapChain4;
		}

		ComPtr<ID3D12DescriptorHeap> create_descriptorHeap(ComPtr<ID3D12Device2> a_device, D3D12_DESCRIPTOR_HEAP_TYPE a_type, uint32_t a_numDescriptors)
		{
			ComPtr<ID3D12DescriptorHeap> descriptorHeap;

			D3D12_DESCRIPTOR_HEAP_DESC desc_descriptorHeap = {};
			desc_descriptorHeap.NumDescriptors = a_numDescriptors;
			desc_descriptorHeap.Type = a_type;

			check_MicrosoftHRESULT(a_device->CreateDescriptorHeap(&desc_descriptorHeap, IID_PPV_ARGS(&descriptorHeap)));

			return descriptorHeap;
		}

		ComPtr<ID3D12CommandAllocator> create_commandAllocator(ComPtr<ID3D12Device2> a_device, D3D12_COMMAND_LIST_TYPE a_type)
		{
			ComPtr<ID3D12CommandAllocator> commandAllocator;
			check_MicrosoftHRESULT(a_device->CreateCommandAllocator(a_type, IID_PPV_ARGS(&commandAllocator)));

			return commandAllocator;
		}

		ComPtr<ID3D12GraphicsCommandList> create_commandList(ComPtr<ID3D12Device2> a_device, ComPtr<ID3D12CommandAllocator> a_commandAllocator, D3D12_COMMAND_LIST_TYPE a_type)
		{
			ComPtr<ID3D12GraphicsCommandList> commandList;
			check_MicrosoftHRESULT(a_device->CreateCommandList(0, a_type, a_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

			check_MicrosoftHRESULT(commandList->Close());

			return commandList;
		}

		ComPtr<ID3D12Fence> create_fence(ComPtr<ID3D12Device2> a_device)
		{
			ComPtr<ID3D12Fence> fence;

			check_MicrosoftHRESULT(a_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

			return fence;
		}

		HANDLE create_eventHandle()
		{
			HANDLE fenceEvent;

			fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			mHardAssert(fenceEvent != NULL);

			return fenceEvent;
		}

		U64 signal_fence(ComPtr<ID3D12CommandQueue> a_commandQueue, ComPtr<ID3D12Fence> a_fence, U64& a_fenceValue)
		{
			U64 fenceValueForSignal = ++a_fenceValue;
			check_MicrosoftHRESULT(a_commandQueue->Signal(a_fence.Get(), fenceValueForSignal));

			return fenceValueForSignal;
		}

		void wait_fenceValue(ComPtr<ID3D12Fence> a_fence, uint64_t a_fenceValue, HANDLE a_fenceEvent, std::chrono::milliseconds a_duration)
		{
			if (a_fence->GetCompletedValue() < a_fenceValue)
			{
				check_MicrosoftHRESULT(a_fence->SetEventOnCompletion(a_fenceValue, a_fenceEvent));
				WaitForSingleObject(a_fenceEvent, static_cast<DWORD>(a_duration.count()));
			}
		}

		void flush(ComPtr<ID3D12CommandQueue> a_commandQueue, ComPtr<ID3D12Fence> a_fence, uint64_t& a_fenceValue, HANDLE a_fenceEvent)
		{
			uint64_t fenceValueForSignal = signal_fence(a_commandQueue, a_fence, a_fenceValue);
			wait_fenceValue(a_fence, fenceValueForSignal, a_fenceEvent);
		}

		void DX12Renderer::init(HWND a_hwnd, U32 a_width, U32 a_height, mBool a_useWarp)
		{
			enable_debugLayer();

			m_tearingSupported = check_tearingSupport();

			m_clientWidth = a_width;
			m_clientHeight = a_height;

			ComPtr<IDXGIAdapter4> dxgiAdapter4 = get_adapter(a_useWarp);

			m_device = create_device(dxgiAdapter4);

			m_commandQueue = create_commandQueue(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);

			m_swapChain = create_swapChain(a_hwnd, m_commandQueue,
				m_clientWidth, m_clientHeight, scm_numFrames);

			m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

			m_RTVDescriptorHeap = create_descriptorHeap(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, scm_numFrames);
			m_RTVDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			update_renderTargetViews(m_device, m_swapChain, m_RTVDescriptorHeap);

			for (Int i = 0; i < scm_numFrames; ++i)
			{
				m_commandAllocators[i] = create_commandAllocator(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
			}
			m_commandList = create_commandList(m_device,
				m_commandAllocators[m_currentBackBufferIndex], D3D12_COMMAND_LIST_TYPE_DIRECT);

			m_fence = create_fence(m_device);
			m_fenceEvent = create_eventHandle();
		}

		void DX12Renderer::deinit()
		{
			flush(m_commandQueue, m_fence, m_fenceValue, m_fenceEvent);

			CloseHandle(m_fenceEvent);
		}

		void DX12Renderer::update_renderTargetViews(ComPtr<ID3D12Device2> a_device, ComPtr<IDXGISwapChain4> a_swapChain, ComPtr<ID3D12DescriptorHeap> a_descriptorHeap)
		{
			UInt size_rtvDescriptor = a_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(a_descriptorHeap->GetCPUDescriptorHandleForHeapStart());

			for (int i = 0; i < scm_numFrames; ++i)
			{
				ComPtr<ID3D12Resource> backBuffer;
				check_MicrosoftHRESULT(a_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

				a_device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

				m_backBuffers[i] = backBuffer;

				rtvHandle.Offset(size_rtvDescriptor);
			}
		}

		void DX12Renderer::render()
		{
			auto commandAllocator = m_commandAllocators[m_currentBackBufferIndex];
			auto backBuffer = m_backBuffers[m_currentBackBufferIndex];

			commandAllocator->Reset();
			m_commandList->Reset(commandAllocator.Get(), nullptr);
			// Clear the render target.
			{
				CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
					backBuffer.Get(),
					D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

				m_commandList->ResourceBarrier(1, &barrier);
				Float clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
				CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(m_RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
					m_currentBackBufferIndex, m_RTVDescriptorSize);

				m_commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
			}
			// Present
			{
				CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
					backBuffer.Get(),
					D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
				m_commandList->ResourceBarrier(1, &barrier);

				check_MicrosoftHRESULT(m_commandList->Close());

				ID3D12CommandList* const commandLists[] = {
					m_commandList.Get()
				};
				m_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

				UINT syncInterval = m_vSync ? 1 : 0;
				UINT presentFlags = m_tearingSupported && !m_vSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
				check_MicrosoftHRESULT(m_swapChain->Present(syncInterval, presentFlags));

				m_frameFenceValues[m_currentBackBufferIndex] = signal_fence(m_commandQueue, m_fence, m_fenceValue);
				m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

				wait_fenceValue(m_fence, m_frameFenceValues[m_currentBackBufferIndex], m_fenceEvent);
			}
		}

		void DX12Renderer::resize(U32 a_width, U32 a_height)
		{
			if (m_clientWidth != a_width || m_clientHeight != a_height)
			{
				// Don't allow 0 size swap chain back buffers.
				m_clientWidth = std::max(1u, a_width);
				m_clientHeight = std::max(1u, a_height);

				flush(m_commandQueue, m_fence, m_fenceValue, m_fenceEvent);

				for (Int i = 0; i < scm_numFrames; ++i)
				{
					// Any references to the back buffers must be released
					// before the swap chain can be resized.
					m_backBuffers[i].Reset();
					m_frameFenceValues[i] = m_frameFenceValues[m_currentBackBufferIndex];
				}

				DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
				check_MicrosoftHRESULT(m_swapChain->GetDesc(&swapChainDesc));
				check_MicrosoftHRESULT(m_swapChain->ResizeBuffers(scm_numFrames, a_width, a_height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

				m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
				update_renderTargetViews(m_device, m_swapChain, m_RTVDescriptorHeap);
			}
		}
	};
};