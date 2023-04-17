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

    void execute_commandList(
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> a_commandList);

    mU64  signal_fence();
    mBool is_fenceComplete(mU64 a_fenceValue);
    void  wait_fenceValue(mU64 a_fenceValue);
    void  flush();

    ComPtr<ID3D12CommandQueue> get_D3D12CommandQueue() const;

   protected:
    ComPtr<ID3D12CommandAllocator> create_commandAllocator();
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> create_commandList(
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> a_allocator);

   private:
    D3D12_COMMAND_LIST_TYPE    m_commandListType;
    ComPtr<ID3D12Device2>      m_d3d12Device;
    ComPtr<ID3D12CommandQueue> m_d3d12CommandQueue;
    ComPtr<ID3D12Fence>        m_d3d12Fence;
    HANDLE                     m_fenceEvent;
    mU64                       m_fenceValue;

    using CommandListQueue = std::queue<ComPtr<ID3D12GraphicsCommandList2>>;

    struct CommandAllocatorEntry
    {
        mU64                           fenceValue;
        ComPtr<ID3D12CommandAllocator> commandAllocator;
        CommandListQueue               availableCommandLists;
        CommandListQueue               usedCommandLists;
    };

    using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;

    CommandAllocatorQueue m_inFlightCommandAllocators;
    CommandAllocatorQueue m_freeCommandAllocators;
};

}  // namespace dx12
}  // namespace m

#endif  // M_DX12COMMAND_QUEUE