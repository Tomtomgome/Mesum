#ifndef M_DX12COMMAND_QUEUE
#define M_DX12COMMAND_QUEUE
#pragma once

#include <DX12RendererCommon.hpp>

namespace m
{
namespace dx12
{

class DX12CommandQueue
{
   public:
    void init(Microsoft::WRL::ComPtr<ID3D12Device2> a_device,
              D3D12_COMMAND_LIST_TYPE               a_type);
    void destroy();
    // Get an available command list from the command queue.
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> get_commandList();

    // Execute a command list.
    // Returns the fence value to wait for for this command list.
    U64 execute_commandList(
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> a_commandList);

    U64  signal_fence();
    Bool is_fenceComplete(U64 a_fenceValue);
    void wait_fenceValue(U64 a_fenceValue);
    void flush();

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> get_D3D12CommandQueue() const;

   protected:
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> create_commandAllocator();
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> create_commandList(
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> a_allocator);

   private:
    // Keep track of command allocators that are "in-flight"
    struct CommandAllocatorEntry
    {
        U64                                            m_fenceValue;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
    };

    using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
    using CommandListQueue =
        std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> >;

    D3D12_COMMAND_LIST_TYPE                    m_commandListType;
    Microsoft::WRL::ComPtr<ID3D12Device2>      m_d3d12Device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_d3d12CommandQueue;
    Microsoft::WRL::ComPtr<ID3D12Fence>        m_d3d12Fence;
    HANDLE                                     m_fenceEvent;
    U64                                        m_fenceValue;

    CommandAllocatorQueue m_commandAllocatorQueue;
    CommandListQueue      m_commandListQueue;
};

}  // namespace dx12
}  // namespace m

#endif  // M_DX12COMMAND_QUEUE