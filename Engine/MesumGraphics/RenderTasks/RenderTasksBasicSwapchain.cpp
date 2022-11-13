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
mTaskSwapchainResize::mTaskSwapchainResize(mTaskDataSwapchainResize* a_data)
{
    mAssert(a_data != nullptr);
    taskData = *a_data;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mTaskSwapchainResize::execute() const
{
    unref_safe(taskData.pSwapchain).resize(taskData.width, taskData.height);
}

#ifdef M_DX12_RENDERER
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mDefine_getForDx12Implementation(mTaskDataSwapchainResize,
                                 mTaskSwapchainResize);
#endif  // M_DX12_RENDERER

#ifdef M_VULKAN_RENDERER
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mDefine_getForVulkanImplementation(mTaskDataSwapchainResize,
                                   mTaskSwapchainResize);
#endif  // M_VULKAN_RENDERER
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
    pOutputRT = dx12::mApi::sm_mal.construct<dx12::mRenderTarget>();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mTaskSwapchainWaitForRTDx12::~mTaskSwapchainWaitForRTDx12()
{
    dx12::mApi::sm_mal.destroy<dx12::mRenderTarget>(
        static_cast<dx12::mRenderTarget*>(pOutputRT));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mTaskSwapchainWaitForRTDx12::execute() const
{
    auto& swapchain = *(static_cast<dx12::mSwapchain*>(taskData.pSwapchain));
    auto& synchTool = *(static_cast<dx12::mSynchTool*>(taskData.pSynchTool));

    mUInt waitIndex             = synchTool.currentFenceIndex;
    synchTool.currentFenceIndex = swapchain.get_currentBackBufferIndex();

    if (!taskData.flush)
    {
        waitIndex = synchTool.currentFenceIndex;
    }

    dx12::DX12Context::gs_dx12Contexte->get_commandQueue().wait_fenceValue(
        synchTool.fenceValues[waitIndex]);

    auto backbuffer = swapchain.get_backbuffer(synchTool.currentFenceIndex);

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

        dx12::mRenderTarget& outputRT =
            unref_safe(static_cast<dx12::mRenderTarget*>(pOutputRT));
        outputRT.rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(
            swapchain.get_rtv(synchTool.currentFenceIndex));

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
    pOutputRT = vulkan::mApi::sm_mal.construct<vulkan::mRenderTarget>();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mTaskSwapchainWaitForRTVulkan::~mTaskSwapchainWaitForRTVulkan()
{
    vulkan::mApi::sm_mal.destroy<vulkan::mRenderTarget>(
        static_cast<vulkan::mRenderTarget*>(pOutputRT));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void mTaskSwapchainWaitForRTVulkan::execute() const
{
    auto& swapchain = *(static_cast<vulkan::mSwapchain*>(taskData.pSwapchain));
    auto& synchTool = *(static_cast<vulkan::mSynchTool*>(taskData.pSynchTool));

    if (!taskData.flush)
    {
        synchTool.currentFenceIndex =
            (synchTool.currentFenceIndex + 1) % synchTool.fenceValues.size();
    }

    mUInt waitIndex = synchTool.currentFenceIndex;

    // Check oldRenderHasFinished for next frame
    vulkan::VulkanContext::get_commandQueue().wait_onFenceValue(
        synchTool.fenceValues[waitIndex]);

    // Aquire next image
    swapchain.acquire_image(waitIndex);

    vulkan::VulkanContext::get_commandQueue().push_waitOnSemaphores(
        {swapchain.get_acquiredImageSemaphore(synchTool.currentFenceIndex)});

    vulkan::mRenderTarget& outputRT =
        unref_safe(static_cast<vulkan::mRenderTarget*>(pOutputRT));
    outputRT = swapchain.get_currentRenderTarget();

    // Transitionning
    auto commandBuffer =
        vulkan::VulkanContext::get_commandQueue().get_commandBuffer();
    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    imageMemoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
    imageMemoryBarrier.newLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageMemoryBarrier.image         = swapchain.get_currentImage();
    imageMemoryBarrier.subresourceRange = {
        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = 0,
        .levelCount     = 1,
        .baseArrayLayer = 0,
        .layerCount     = 1,
    };

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,              // srcStageMask
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // dstStageMask
        0, 0, nullptr, 0, nullptr,
        1,                   // imageMemoryBarrierCount
        &imageMemoryBarrier  // pImageMemoryBarriers
    );

    vulkan::VulkanContext::get_commandQueue().submit_commandBuffer(
        commandBuffer);
}

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
    auto& swapchain = *(static_cast<dx12::mSwapchain*>(taskData.pSwapchain));
    auto& synchTool = *(static_cast<dx12::mSynchTool*>(taskData.pSynchTool));

    dx12::ComPtr<ID3D12GraphicsCommandList2> graphicCommandList =
        dx12::DX12Context::gs_dx12Contexte->get_commandQueue()
            .get_commandList();

    auto backbuffer = swapchain.get_backbuffer(synchTool.currentFenceIndex);
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

    synchTool.fenceValues[synchTool.currentFenceIndex] =
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
void mTaskSwapchainPresentVulkan::execute() const
{
    auto& swapchain = *(static_cast<vulkan::mSwapchain*>(taskData.pSwapchain));
    auto& synchTool = *(static_cast<vulkan::mSynchTool*>(taskData.pSynchTool));

    // Transition
    auto commandBuffer =
        vulkan::VulkanContext::get_commandQueue().get_commandBuffer();
    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    imageMemoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageMemoryBarrier.newLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    imageMemoryBarrier.image         = swapchain.get_currentImage();
    imageMemoryBarrier.subresourceRange = {
        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = 0,
        .levelCount     = 1,
        .baseArrayLayer = 0,
        .layerCount     = 1,
    };

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // srcStageMask
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           // dstStageMask
        0, 0, nullptr, 0, nullptr,
        1,                   // imageMemoryBarrierCount
        &imageMemoryBarrier  // pImageMemoryBarriers
    );

    vulkan::VulkanContext::get_commandQueue().submit_commandBuffer(
        commandBuffer);

    // Signal
    synchTool.fenceValues[synchTool.currentFenceIndex] =
        vulkan::VulkanContext::get_commandQueue().signal_fence();

    vulkan::VulkanContext::get_commandQueue().signal_semaphores(
        {swapchain.get_renderCompletedSemaphore(synchTool.currentFenceIndex)});

    // Presentation
    swapchain.present(synchTool.currentFenceIndex);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mDefine_getForVulkanImplementation(mTaskDataSwapchainPresent,
                                   mTaskSwapchainPresentVulkan);
#endif  // M_VULKAN_RENDERER

}  // namespace m::render