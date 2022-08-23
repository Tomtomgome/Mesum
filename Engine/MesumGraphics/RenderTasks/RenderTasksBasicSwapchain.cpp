#include "RenderTasksBasicSwapchain.hpp"

#include <Kernel/Kernel.hpp>

namespace m::render
{
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mTaskSwapchainWaitForRT::mTaskSwapchainWaitForRT(
    mTaskDataSwapchainWaitForRT* a_data)
{
    mAssert(a_data != nullptr);
    taskData = *a_data;
}

#ifdef M_DX12_RENDERER
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mTaskSwapchainWaitForRTDx12::mTaskSwapchainWaitForRTDx12(
    mTaskDataSwapchainWaitForRT* a_data)
    : mTaskSwapchainWaitForRT(a_data)
{
    pOutputRT = dx12::mApiDX12::sm_mal.construct<dx12::mRenderTargetDX12>();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mTaskSwapchainWaitForRTDx12::~mTaskSwapchainWaitForRTDx12()
{
    dx12::mApiDX12::sm_mal.destroy<dx12::mRenderTargetDX12>(
        static_cast<dx12::mRenderTargetDX12*>(pOutputRT));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mTaskSwapchainWaitForRTDx12::execute() const
{
    auto& swapchain =
        *(static_cast<dx12::mSwapchainDX12*>(taskData.pSwapchain));
    auto& shynchTool =
        *(static_cast<dx12::mSynchToolDX12*>(taskData.pSynchTool));

    shynchTool.currentFenceIndex = swapchain.get_currentBackBufferIndex();

    dx12::DX12Context::gs_dx12Contexte->get_commandQueue().wait_fenceValue(
        shynchTool.fenceValues[shynchTool.currentFenceIndex]);

    auto backbuffer = swapchain.get_backbuffer(shynchTool.currentFenceIndex);

    dx12::ComPtr<ID3D12GraphicsCommandList2> graphicCommandList =
        dx12::DX12Context::gs_dx12Contexte->get_commandQueue()
            .get_commandList();
    // Clear the render target.
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backbuffer, D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET);

        graphicCommandList->ResourceBarrier(1, &barrier);

        mFloat clearColor[] = {0.4f, 0.6f, 0.9f, 0.0f};

        dx12::mRenderTargetDX12& outputRT =
            unref_safe(static_cast<dx12::mRenderTargetDX12*>(pOutputRT));
        outputRT.rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(swapchain.get_rtv(shynchTool.currentFenceIndex));

        graphicCommandList->ClearRenderTargetView(outputRT.rtv, clearColor, 0,
                                                  nullptr);

        dx12::DX12Context::gs_dx12Contexte->get_commandQueue()
            .execute_commandList(graphicCommandList);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mDefine_getForDx12Implementation(mTaskDataSwapchainWaitForRT,
                                 mTaskSwapchainWaitForRTDx12);

#endif  // M_DX12_RENDERER

#ifdef M_VULKAN_RENDERER
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mTaskSwapchainWaitForRTVulkan::mTaskSwapchainWaitForRTVulkan(
    mTaskDataSwapchainWaitForRT* a_data)
    : mTaskSwapchainWaitForRT(a_data)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mTaskSwapchainWaitForRTVulkan::execute() const {}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mDefine_getForVulkanImplementation(mTaskDataSwapchainWaitForRT,
                                   mTaskSwapchainWaitForRTVulkan);
#endif  // M_VULKAN_RENDERER

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mTaskSwapchainPresent::mTaskSwapchainPresent(mTaskDataSwapchainPresent* a_data)
{
    mAssert(a_data != nullptr);
    taskData = *a_data;
}

#ifdef M_DX12_RENDERER
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mTaskSwapchainPresentDx12::mTaskSwapchainPresentDx12(
    mTaskDataSwapchainPresent* a_data)
    : mTaskSwapchainPresent(a_data)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mTaskSwapchainPresentDx12::execute() const
{
    auto& swapchain =
        *(static_cast<dx12::mSwapchainDX12*>(taskData.pSwapchain));
    auto& shynchTool =
        *(static_cast<dx12::mSynchToolDX12*>(taskData.pSynchTool));

    dx12::ComPtr<ID3D12GraphicsCommandList2> graphicCommandList =
        dx12::DX12Context::gs_dx12Contexte->get_commandQueue()
            .get_commandList();

    auto backbuffer = swapchain.get_backbuffer(shynchTool.currentFenceIndex);
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    graphicCommandList->ResourceBarrier(1, &barrier);

    dx12::DX12Context::gs_dx12Contexte->get_commandQueue().execute_commandList(
        graphicCommandList);

    // UINT syncInterval = get_syncInterval();  // m_vSync ? 1 : 0;
    // UINT presentFlags = get_presentFlags();  // m_tearingSupported &&
    // !m_vSync ?
    //  DXGI_PRESENT_ALLOW_TEARING : 0;
    // check_mhr(m_pSwapChain->Present(syncInterval, presentFlags));
    dx12::check_mhr(swapchain.get_swapchain()->Present(1, 0));

    shynchTool.fenceValues[shynchTool.currentFenceIndex] =
        dx12::DX12Context::gs_dx12Contexte->get_commandQueue().signal_fence();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mDefine_getForDx12Implementation(mTaskDataSwapchainPresent,
                                 mTaskSwapchainPresentDx12);

#endif  // M_DX12_RENDERER

#ifdef M_VULKAN_RENDERER
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mTaskSwapchainPresentVulkan::mTaskSwapchainPresentVulkan(
    mTaskDataSwapchainPresent* a_data)
    : mTaskSwapchainPresent(a_data)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mTaskSwapchainPresentVulkan::execute() const {}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mDefine_getForVulkanImplementation(mTaskDataSwapchainPresent,
                                   mTaskSwapchainPresentVulkan);
#endif  // M_VULKAN_RENDERER

}  // namespace m::render