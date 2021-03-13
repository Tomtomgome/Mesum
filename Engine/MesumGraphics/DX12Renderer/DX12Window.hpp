#ifndef M_DX12WINDOW
#define M_DX12WINDOW
#pragma once

#include <MesumGraphics/DX12Renderer/DX12Renderer.hpp>

namespace m
{
namespace dx12
{

class DX12Window
{
   public:
    void init(HWND a_hwnd, U32 a_width, U32 a_height);
    void destroy();

    void render();

    void resize(U32 a_width, U32 a_height);

    UInt get_syncInterval() { return m_vSync ? 1 : 0; }

    UInt get_presentFlags()
    {
        return m_tearingSupported && !m_vSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    }

   private:
    void update_renderTargetViews(
        ComPtr<ID3D12Device2> a_device, ComPtr<IDXGISwapChain4> a_swapChain,
        ComPtr<ID3D12DescriptorHeap> a_descriptorHeap);

    // The number of swap chain back buffers.
    static const U8 scm_numFrames = 3;

    // By default, enable V-Sync.
    // Can be toggled with the V key.
    Bool m_vSync            = true;
    Bool m_tearingSupported = false;

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

}  // namespace dx12
}  // namespace m
#endif  // M_DX12WINDOW