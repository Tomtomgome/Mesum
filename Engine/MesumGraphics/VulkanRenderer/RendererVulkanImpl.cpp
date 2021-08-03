#include <imgui_impl_vulkan.h>

#include <RendererVulkanImpl.hpp>
#include <VulkanContext.hpp>
#include <limits>

namespace m::vulkan
{
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
render::Task* VulkanRenderTaskset::add_task(render::TaskData* a_data)
{
    auto task = a_data->GetVulkanImplementation(a_data);
    m_set_tasks.push_back(task);
    return task;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanRenderTaskset::clear()
{
    for (auto task : m_set_tasks) { delete task; }
    m_set_tasks.clear();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanSurface::init_win32(render::Win32SurfaceInitData& a_data)
{
#ifdef M_WIN32
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd      = a_data.m_hwnd;
    createInfo.hinstance = GetModuleHandle(nullptr);

    if (vkCreateWin32SurfaceKHR(VulkanContext::get_instance(), &createInfo,
                                nullptr, &m_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
    m_clientWidth  = a_data.m_width;
    m_clientHeight = a_data.m_height;
#else
    // Wrong platform
    mInterrupt;
#endif
    init_internal();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanSurface::init_x11(render::X11SurfaceInitData& a_data)
{
#ifdef M_UNIX
    m_clientWidth  = a_data.m_width;
    m_clientHeight = a_data.m_height;
#else
    // Wrong platform
    mInterrupt;
#endif
    init_internal();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanSurface::set_asDearImGuiSurface()
{
    m_isHoldingDearImgui = true;

    {
        VkDescriptorPoolSize pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType   = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags   = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes    = pool_sizes;
        vkCreateDescriptorPool(VulkanContext::get_logDevice(), &pool_info,
                               nullptr, &m_dearImGuiDescriptorPool);
    }

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = VulkanContext::get_instance();
    init_info.PhysicalDevice            = VulkanContext::get_physDevice();
    init_info.Device                    = VulkanContext::get_logDevice();
    init_info.QueueFamily     = VulkanContext::get_graphicQueueFamilyIndex();
    init_info.Queue           = VulkanContext::get_graphicQueue();
    init_info.PipelineCache   = VK_NULL_HANDLE;
    init_info.DescriptorPool  = m_dearImGuiDescriptorPool;
    init_info.Allocator       = nullptr;
    init_info.MinImageCount   = scm_numFrames;
    init_info.ImageCount      = scm_numFrames;
    init_info.CheckVkResultFn = check_vkResult;
    ImGui_ImplVulkan_Init(&init_info, m_mainRenderPass);

    VkCommandBuffer command_buffer =
        VulkanContext::gs_VulkanContexte->get_singleUseCommandBuffer();

    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

    VulkanContext::gs_VulkanContexte->submit_signleUseCommandBuffer(
        command_buffer);

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
render::Taskset* addNew_renderTaskset()
{
    return nullptr;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanSurface::render()
{
    m_currentBackBufferIndex = (m_currentBackBufferIndex + 1) % scm_numFrames;

    // Check oldRenderHasFinished for next frame
    VulkanContext::wait_onMainTimelineTstp(
        m_tstpRenderFinish[m_currentBackBufferIndex]);

    // Aquire next image
    UInt imageIndex;
    vkAcquireNextImageKHR(VulkanContext::get_logDevice(), m_swapChain,
                          std::numeric_limits<U64>::max(),
                          m_semaphoresImageAcquired[m_currentBackBufferIndex],
                          VK_NULL_HANDLE, &imageIndex);

    {
        check_vkResult(vkResetCommandPool(
            VulkanContext::get_logDevice(),
            m_frameCommandPools[m_currentBackBufferIndex], 0));
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        check_vkResult(vkBeginCommandBuffer(
            m_frameMainCommandBuffers[m_currentBackBufferIndex], &info));
    }

    {
        VkRenderPassBeginInfo info   = {};
        info.sType                   = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass              = m_mainRenderPass;
        info.framebuffer             = m_frameFramebuffers[imageIndex];
        info.renderArea.extent.width = m_clientWidth;
        info.renderArea.extent.height = m_clientHeight;
        VkClearValue clearValues[1]   = {};
        clearValues[0].color          = {0.4f, 0.6f, 0.9f, 1.0f};
        if (!m_isHoldingDearImgui)
        {
            clearValues[0].color.float32[0] = 0.9f;
            clearValues[0].color.float32[2] = 0.4f;
        }
        info.clearValueCount = 1;
        info.pClearValues    = clearValues;
        vkCmdBeginRenderPass(
            m_frameMainCommandBuffers[m_currentBackBufferIndex], &info,
            VK_SUBPASS_CONTENTS_INLINE);
    }

    if (m_isHoldingDearImgui)
    {
        // Record Imgui Draw Data and draw funcs into command buffer
        ImGui_ImplVulkan_RenderDrawData(
            ImGui::GetDrawData(),
            m_frameMainCommandBuffers[m_currentBackBufferIndex]);
    }
    // Submit command buffer
    vkCmdEndRenderPass(m_frameMainCommandBuffers[m_currentBackBufferIndex]);

    check_vkResult(vkEndCommandBuffer(
        m_frameMainCommandBuffers[m_currentBackBufferIndex]));

    m_tstpRenderFinish[m_currentBackBufferIndex] =
        VulkanContext::submit_onMainTimeline(
            m_frameMainCommandBuffers[m_currentBackBufferIndex],
            {m_semaphoresImageAcquired[m_currentBackBufferIndex]},
            {m_semaphoresRenderCompleted[m_currentBackBufferIndex]});

    // Presentation
    VkPresentInfoKHR infoPresent   = {};
    infoPresent.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    infoPresent.waitSemaphoreCount = 1;
    infoPresent.pWaitSemaphores =
        &m_semaphoresRenderCompleted[m_currentBackBufferIndex];
    infoPresent.swapchainCount = 1;
    infoPresent.pSwapchains    = &m_swapChain;
    infoPresent.pImageIndices  = &imageIndex;
    VulkanContext::present(infoPresent);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanSurface::resize(U32 a_width, U32 a_height)
{
    if (m_clientWidth != a_width || m_clientHeight != a_height)
    {
        // Don't allow 0 size swap chain back buffers.
        m_clientWidth  = std::max(1u, a_width);
        m_clientHeight = std::max(1u, a_height);
        VulkanContext::wait_onMainTimelineTstp(
            m_tstpRenderFinish[m_currentBackBufferIndex]);
        destroy_swapChain();
        init_swapChain();
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanSurface::destroy()
{
    VulkanContext::wait_onMainTimelineTstp(
        m_tstpRenderFinish[m_currentBackBufferIndex]);

    if (m_isHoldingDearImgui)
    {
        ImGui_ImplVulkan_Shutdown();
    }

    if (m_isHoldingDearImgui)
    {
        vkDestroyDescriptorPool(VulkanContext::get_logDevice(),
                                m_dearImGuiDescriptorPool, nullptr);
    }

    for (auto& m_frameCommandPool : m_frameCommandPools)
    {
        vkDestroyCommandPool(VulkanContext::get_logDevice(), m_frameCommandPool,
                             nullptr);
    }

    destroy_swapChain();

    vkDestroyRenderPass(VulkanContext::get_logDevice(), m_mainRenderPass,
                        nullptr);

    for (size_t i = 0; i < scm_numFrames; i++)
    {
        vkDestroySemaphore(VulkanContext::get_logDevice(),
                           m_semaphoresImageAcquired[i], nullptr);
        vkDestroySemaphore(VulkanContext::get_logDevice(),
                           m_semaphoresRenderCompleted[i], nullptr);
    }

    vkDestroySurfaceKHR(VulkanContext::get_instance(), m_surface, nullptr);
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
void VulkanSurface::init_internal()
{
    U32 queueFamilyIndex;
    find_graphicQueueFamilyIndex(VulkanContext::get_physDevice(),
                                 queueFamilyIndex);

    VkBool32 surfaceSupported;
    vkGetPhysicalDeviceSurfaceSupportKHR(VulkanContext::get_physDevice(),
                                         queueFamilyIndex, m_surface,
                                         &surfaceSupported);
    if (!surfaceSupported)
    {
        throw std::runtime_error(
            "Surfaces should be supported by the application");
    }

    std::vector<VkSurfaceFormatKHR> supportedFormats;

    U32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanContext::get_physDevice(),
                                         m_surface, &formatCount, nullptr);

    if (formatCount == 0)
    {
        throw std::runtime_error("Need to support at least some formats");
    }

    supportedFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanContext::get_physDevice(),
                                         m_surface, &formatCount,
                                         supportedFormats.data());

    Bool isFormatSupported = false;
    for (size_t i = 0; i < formatCount; i++)
    {
        if (supportedFormats[i].format == scm_selectedSwapChainFormat)
        {
            isFormatSupported = true;
            break;
        }
    }

    if (!isFormatSupported)
    {
        throw std::runtime_error(
            "VK_FORMAT_B8G8R8A8_UNORM Needs to be supported at the moment");
    }

    m_tstpRenderFinish.resize(scm_numFrames, 0);

    m_semaphoresImageAcquired.resize(scm_numFrames);
    m_semaphoresRenderCompleted.resize(scm_numFrames);

    VkSemaphoreCreateInfo createSemaphore = {};
    createSemaphore.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    createSemaphore.flags = 0;

    for (size_t i = 0; i < scm_numFrames; i++)
    {
        if (vkCreateSemaphore(VulkanContext::get_logDevice(), &createSemaphore,
                              nullptr,
                              &m_semaphoresImageAcquired[i]) != VK_SUCCESS ||
            vkCreateSemaphore(VulkanContext::get_logDevice(), &createSemaphore,
                              nullptr,
                              &m_semaphoresRenderCompleted[i]) != VK_SUCCESS)
        {
            throw std::runtime_error(
                "failed to create semaphores for a frame!");
        }
    }

    // Create the Render Pass

    VkAttachmentDescription attachment     = {};
    attachment.format                      = scm_selectedSwapChainFormat;
    attachment.samples                     = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp                      = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp                     = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp               = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp              = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout               = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout                 = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference color_attachment = {};
    color_attachment.attachment            = 0;
    color_attachment.layout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass   = {};
    subpass.pipelineBindPoint      = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount   = 1;
    subpass.pColorAttachments      = &color_attachment;
    VkSubpassDependency dependency = {};
    dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass          = 0;
    dependency.srcStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask    = 0;
    dependency.dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    VkRenderPassCreateInfo info = {};
    info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount        = 1;
    info.pAttachments           = &attachment;
    info.subpassCount           = 1;
    info.pSubpasses             = &subpass;
    info.dependencyCount        = 1;
    info.pDependencies          = &dependency;
    vkCreateRenderPass(VulkanContext::get_logDevice(), &info, nullptr,
                       &m_mainRenderPass);

    init_swapChain();

    for (int i = 0; i < scm_numFrames; i++)
    {
        VkCommandPoolCreateInfo createCommandPool = {};
        createCommandPool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        createCommandPool.queueFamilyIndex =
            VulkanContext::get_graphicQueueFamilyIndex();

        check_vkResult(vkCreateCommandPool(VulkanContext::get_logDevice(),
                                           &createCommandPool, nullptr,
                                           &m_frameCommandPools[i]));

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_frameCommandPools[i];
        allocInfo.commandBufferCount = 1;

        check_vkResult(vkAllocateCommandBuffers(VulkanContext::get_logDevice(),
                                                &allocInfo,
                                                &m_frameMainCommandBuffers[i]));
    }
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
void VulkanSurface::init_swapChain()
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanContext::get_physDevice(),
                                              m_surface, &surfaceCapabilities);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = m_surface;
    createInfo.minImageCount    = scm_numFrames;
    createInfo.imageFormat      = scm_selectedSwapChainFormat;
    createInfo.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent      = {m_clientWidth, m_clientHeight};
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform     = surfaceCapabilities.currentTransform;
    createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped          = VK_TRUE;
    createInfo.oldSwapchain     = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(VulkanContext::get_logDevice(), &createInfo,
                             nullptr, &m_swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    U32 imageCount;
    vkGetSwapchainImagesKHR(VulkanContext::get_logDevice(), m_swapChain,
                            &imageCount, nullptr);
    m_swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(VulkanContext::get_logDevice(), m_swapChain,
                            &imageCount, m_swapChainImages.data());

    m_swapChainImageViews.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++)
    {
        VkImageViewCreateInfo createInfoImageViews{};
        createInfoImageViews.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfoImageViews.image = m_swapChainImages[i];
        createInfoImageViews.viewType     = VK_IMAGE_VIEW_TYPE_2D;
        createInfoImageViews.format       = scm_selectedSwapChainFormat;
        createInfoImageViews.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfoImageViews.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfoImageViews.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfoImageViews.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfoImageViews.subresourceRange.aspectMask =
            VK_IMAGE_ASPECT_COLOR_BIT;
        createInfoImageViews.subresourceRange.baseMipLevel   = 0;
        createInfoImageViews.subresourceRange.levelCount     = 1;
        createInfoImageViews.subresourceRange.baseArrayLayer = 0;
        createInfoImageViews.subresourceRange.layerCount     = 1;
        if (vkCreateImageView(VulkanContext::get_logDevice(),
                              &createInfoImageViews, nullptr,
                              &m_swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }

        VkCommandBuffer commandBuffer =
            VulkanContext::gs_VulkanContexte->get_singleUseCommandBuffer();

        VkImageMemoryBarrier barrier{};
        barrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout           = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image               = m_swapChainImages[i];
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                             VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0, nullptr,
                             0, nullptr, 1, &barrier);

        VulkanContext::gs_VulkanContexte->submit_signleUseCommandBuffer(
            commandBuffer);
    }

    m_frameFramebuffers.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++)
    {
        VkImageView attachments[] = {m_swapChainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType      = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_mainRenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments    = attachments;
        framebufferInfo.width           = m_clientWidth;
        framebufferInfo.height          = m_clientHeight;
        framebufferInfo.layers          = 1;

        if (vkCreateFramebuffer(VulkanContext::get_logDevice(),
                                &framebufferInfo, nullptr,
                                &m_frameFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
void VulkanSurface::destroy_swapChain()
{
    for (Int i = 0; i < scm_numFrames; ++i)
    {
        m_tstpRenderFinish[i] = m_tstpRenderFinish[m_currentBackBufferIndex];
    }

    for (auto framebuffer : m_frameFramebuffers)
    {
        vkDestroyFramebuffer(VulkanContext::get_logDevice(), framebuffer,
                             nullptr);
    }

    for (auto imageView : m_swapChainImageViews)
    {
        vkDestroyImageView(VulkanContext::get_logDevice(), imageView, nullptr);
    }

    vkDestroySwapchainKHR(VulkanContext::get_logDevice(), m_swapChain, nullptr);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanRenderer::init()
{
    VulkanContext::gs_VulkanContexte = new VulkanContext();
    VulkanContext::gs_VulkanContexte->init();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanRenderer::destroy()
{
    VulkanContext::gs_VulkanContexte->deinit();
    delete VulkanContext::gs_VulkanContexte;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanRenderer::start_dearImGuiNewFrameRenderer() const
{
    ImGui_ImplVulkan_NewFrame();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
render::ISurface* VulkanRenderer::get_newSurface()
{
    return new VulkanSurface();
}
}  // namespace m::vulkan