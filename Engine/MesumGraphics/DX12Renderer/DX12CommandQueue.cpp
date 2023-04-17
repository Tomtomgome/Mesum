#include <DX12CommandQueue.hpp>

namespace m
{
namespace dx12
{
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
void DX12CommandQueue::init(ComPtr<ID3D12Device2>   a_device,
                            D3D12_COMMAND_LIST_TYPE a_type)
{
    m_fenceValue      = 0U;
    m_commandListType = a_type;
    m_d3d12Device     = a_device;

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type                     = a_type;
    desc.Priority                 = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask                 = 0;

    check_mhr(m_d3d12Device->CreateCommandQueue(
        &desc, IID_PPV_ARGS(&m_d3d12CommandQueue)));
    mD3D12DebugNamed(m_d3d12CommandQueue, "DX12 CommandQueue");

    check_mhr(m_d3d12Device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE,
                                         IID_PPV_ARGS(&m_d3d12Fence)));
    mD3D12DebugNamed(m_d3d12Fence, "CommandQueue Fence");

    m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    mSoftAssert(m_fenceEvent);
}

void DX12CommandQueue::destroy()
{
    flush();
    CloseHandle(m_fenceEvent);
}
// Get an available command list from the command queue.
Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>
DX12CommandQueue::get_commandList()
{
    ComPtr<ID3D12GraphicsCommandList2> commandList;
    CommandAllocatorEntry*             pCommandAllocatorEntry = nullptr;

    if (m_freeCommandAllocators.empty())
    {
        CommandAllocatorEntry createdCommandAllocator;
        createdCommandAllocator.commandAllocator = create_commandAllocator();
        m_freeCommandAllocators.push(createdCommandAllocator);
    }

    pCommandAllocatorEntry = &m_freeCommandAllocators.front();
    CommandAllocatorEntry& commandAllocatorEntry = *pCommandAllocatorEntry;

    if (!commandAllocatorEntry.availableCommandLists.empty())
    {
        commandList = commandAllocatorEntry.availableCommandLists.front();
        commandAllocatorEntry.availableCommandLists.pop();
    }
    else
    {
        commandList =
            create_commandList(commandAllocatorEntry.commandAllocator);
    }
    commandAllocatorEntry.usedCommandLists.push(commandList);

    check_mhr(commandList->Reset(commandAllocatorEntry.commandAllocator.Get(),
                                  nullptr));

    return commandList;
}

// Execute a command list.
// Returns the fence value to wait for for this command list.
void DX12CommandQueue::execute_commandList(
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> a_commandList)
{
    check_mhr(a_commandList->Close());

    ID3D12CommandList* const ppCommandLists[] = {a_commandList.Get()};

    m_d3d12CommandQueue->ExecuteCommandLists(1, ppCommandLists);
}

mU64 DX12CommandQueue::signal_fence()
{
    uint64_t signalValue =
        dx12::signal_fence(m_d3d12CommandQueue, m_d3d12Fence, m_fenceValue);

    if (!m_freeCommandAllocators.empty())
    {
        m_freeCommandAllocators.front().fenceValue = signalValue;
        m_inFlightCommandAllocators.push(m_freeCommandAllocators.front());
        m_freeCommandAllocators.pop();
    }

    return signalValue;
}

mBool DX12CommandQueue::is_fenceComplete(mU64 a_fenceValue)
{
    return m_d3d12Fence->GetCompletedValue() >= a_fenceValue;
}
void DX12CommandQueue::wait_fenceValue(mU64 a_fenceValue)
{
    dx12::wait_fenceValue(m_d3d12Fence, a_fenceValue, m_fenceEvent);

    // Free/Reset associatedCommandPool;
    while (!m_inFlightCommandAllocators.empty() &&
           m_inFlightCommandAllocators.front().fenceValue <= a_fenceValue)
    {
        CommandAllocatorEntry& entry = m_inFlightCommandAllocators.front();
        check_mhr(entry.commandAllocator->Reset());

        while (!entry.usedCommandLists.empty())
        {
            entry.availableCommandLists.push(entry.usedCommandLists.front());
            entry.usedCommandLists.pop();
        }

        m_freeCommandAllocators.push(m_inFlightCommandAllocators.front());
        m_inFlightCommandAllocators.pop();
    }
}

void DX12CommandQueue::flush()
{
    uint64_t fenceValueForSignal = signal_fence();
    wait_fenceValue(fenceValueForSignal);
}

Microsoft::WRL::ComPtr<ID3D12CommandQueue>
DX12CommandQueue::get_D3D12CommandQueue() const
{
    return m_d3d12CommandQueue;
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator>
DX12CommandQueue::create_commandAllocator()
{
    return dx12::create_commandAllocator(m_d3d12Device, m_commandListType);
}
Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2>
DX12CommandQueue::create_commandList(
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> a_allocator)
{
    return dx12::create_commandList(m_d3d12Device, a_allocator,
                                    m_commandListType);
}

}  // namespace dx12
}  // namespace m