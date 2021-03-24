#include <DX12CommandQueue.hpp>

namespace m
{
namespace dx12
{
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
void DX12CommandQueue::init(ComPtr<ID3D12Device2> a_device,
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

    check_MicrosoftHRESULT(m_d3d12Device->CreateCommandQueue(
        &desc, IID_PPV_ARGS(&m_d3d12CommandQueue)));
    mD3D12DebugNamed(m_d3d12CommandQueue, "DX12 CommandQueue");

    check_MicrosoftHRESULT(m_d3d12Device->CreateFence(
        m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_d3d12Fence)));
    mD3D12DebugNamed(m_d3d12Fence, "CommandQueue Fence");

    m_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    mAssert(m_fenceEvent);
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
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>     commandAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> commandList;
    if (!m_commandAllocatorQueue.empty() &&
        is_fenceComplete(m_commandAllocatorQueue.front().m_fenceValue))
    {
        commandAllocator = m_commandAllocatorQueue.front().m_commandAllocator;
        m_commandAllocatorQueue.pop();

        check_MicrosoftHRESULT(commandAllocator->Reset());
    }
    else
    {
        commandAllocator = create_commandAllocator();
        mD3D12DebugNamed(commandAllocator, "Queued command allocator");
    }

    if (!m_commandListQueue.empty())
    {
        commandList = m_commandListQueue.front();
        m_commandListQueue.pop();
    }
    else
    {
        commandList = create_commandList(commandAllocator);
        mD3D12DebugNamed(commandList, "Queued command list");
    }

    check_MicrosoftHRESULT(commandList->Reset(commandAllocator.Get(), nullptr));

    // Associate the command allocator with the command list so that it can be
    // retrieved when the command list is executed.
    check_MicrosoftHRESULT(commandList->SetPrivateDataInterface(
        __uuidof(ID3D12CommandAllocator), commandAllocator.Get()));
    return commandList;
}

// Execute a command list.
// Returns the fence value to wait for for this command list.
U64 DX12CommandQueue::execute_commandList(
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> a_commandList)
{
    check_MicrosoftHRESULT(a_commandList->Close());

    ID3D12CommandAllocator* commandAllocator;
    UINT                    dataSize = sizeof(commandAllocator);
    check_MicrosoftHRESULT(a_commandList->GetPrivateData(
        __uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));

    ID3D12CommandList* const ppCommandLists[] = {a_commandList.Get()};

    m_d3d12CommandQueue->ExecuteCommandLists(1, ppCommandLists);
    uint64_t fenceValue = signal_fence();

    m_commandAllocatorQueue.emplace(
        CommandAllocatorEntry{fenceValue, commandAllocator});
    m_commandListQueue.push(a_commandList);

    // The ownership of the command allocator has been transferred to the ComPtr
    // in the command allocator queue. It is safe to release the reference
    // in this temporary COM pointer here.
    commandAllocator->Release();

    return fenceValue;
}

U64 DX12CommandQueue::signal_fence()
{
    return dx12::signal_fence(m_d3d12CommandQueue, m_d3d12Fence, m_fenceValue);
}

Bool DX12CommandQueue::is_fenceComplete(U64 a_fenceValue)
{
    return m_d3d12Fence->GetCompletedValue() >= a_fenceValue;
}
void DX12CommandQueue::wait_fenceValue(U64 a_fenceValue)
{
    dx12::wait_fenceValue(m_d3d12Fence, a_fenceValue, m_fenceEvent);
}

void DX12CommandQueue::flush()
{
    dx12::flush(m_d3d12CommandQueue, m_d3d12Fence, m_fenceValue, m_fenceEvent);
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