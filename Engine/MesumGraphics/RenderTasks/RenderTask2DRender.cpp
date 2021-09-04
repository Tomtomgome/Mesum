#include <RenderTask2DRender.hpp>

#include <array>

namespace m::render
{
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Task2dRender::Task2dRender(TaskData2dRender* a_data)
{
    mAssert(a_data != nullptr);
    m_taskData = *a_data;
}

#ifdef M_DX12_RENDERER
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Dx12Task2dRender::Dx12Task2dRender(TaskData2dRender* a_data)
    : Task2dRender(a_data)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

    dx12::ComPtr<ID3DBlob> vs =
        dx12::compile_shader("data/squareShader.hlsl", "vs_main", "vs_6_0");
    dx12::ComPtr<ID3DBlob> ps =
        dx12::compile_shader("data/squareShader.hlsl", "ps_main", "ps_6_0");

    const D3D12_INPUT_ELEMENT_DESC inputElements[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
         D3D12_APPEND_ALIGNED_ELEMENT,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
         D3D12_APPEND_ALIGNED_ELEMENT,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

    pipelineDesc.InputLayout.pInputElementDescs = inputElements;
    pipelineDesc.InputLayout.NumElements        = std::size(inputElements);
    pipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;

    pipelineDesc.VS.BytecodeLength  = vs->GetBufferSize();
    pipelineDesc.VS.pShaderBytecode = vs->GetBufferPointer();
    pipelineDesc.PS.BytecodeLength  = ps->GetBufferSize();
    pipelineDesc.PS.pShaderBytecode = ps->GetBufferPointer();

    pipelineDesc.NumRenderTargets = 1;
    pipelineDesc.RTVFormats[0]    = DXGI_FORMAT_B8G8R8A8_UNORM;
    pipelineDesc.BlendState       = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pipelineDesc.BlendState.RenderTarget[0].BlendEnable = 1;
    pipelineDesc.BlendState.RenderTarget[0].SrcBlend    = D3D12_BLEND_SRC_ALPHA;
    pipelineDesc.BlendState.RenderTarget[0].DestBlend =
        D3D12_BLEND_INV_SRC_ALPHA;
    pipelineDesc.BlendState.RenderTarget[0].SrcBlendAlpha =
        D3D12_BLEND_SRC_ALPHA;
    pipelineDesc.BlendState.RenderTarget[0].DestBlendAlpha =
        D3D12_BLEND_INV_SRC_ALPHA;
    pipelineDesc.BlendState.RenderTarget[0].BlendOp      = D3D12_BLEND_OP_ADD;
    pipelineDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

    pipelineDesc.SampleMask = 0xFFFFFFFF;

    pipelineDesc.RasterizerState.SlopeScaledDepthBias  = 0;
    pipelineDesc.RasterizerState.DepthClipEnable       = false;
    pipelineDesc.RasterizerState.MultisampleEnable     = false;
    pipelineDesc.RasterizerState.AntialiasedLineEnable = false;
    pipelineDesc.RasterizerState.FillMode              = D3D12_FILL_MODE_SOLID;
    pipelineDesc.RasterizerState.CullMode              = D3D12_CULL_MODE_NONE;
    pipelineDesc.RasterizerState.FrontCounterClockwise = true;
    pipelineDesc.RasterizerState.DepthBias             = 0;
    pipelineDesc.RasterizerState.DepthBiasClamp        = 0.0;

    pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineDesc.SampleDesc.Count      = 1;

    CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
    descRootSignature.Init(
        0, nullptr, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    dx12::ComPtr<ID3DBlob> rootBlob;
    dx12::ComPtr<ID3DBlob> errorBlob;
    HRESULT                res;
    res = D3D12SerializeRootSignature(&descRootSignature,
                                      D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob,
                                      &errorBlob);

    if (FAILED(res))
    {
        if (errorBlob != nullptr)
        {
            mLOG((char*)errorBlob->GetBufferPointer());
        }
    }
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;

    dx12::check_MicrosoftHRESULT(device->CreateRootSignature(
        0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature)));

    pipelineDesc.pRootSignature = m_rootSignature.Get();

    dx12::check_MicrosoftHRESULT(device->CreateGraphicsPipelineState(
        &pipelineDesc, IID_PPV_ARGS(&m_pso)));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12Task2dRender::execute() const
{
    dx12::ComPtr<ID3D12GraphicsCommandList2> graphicCommandList =
        dx12::DX12Context::gs_dx12Contexte->get_commandQueue()
            .get_commandList();

    auto currentSurface =
        static_cast<dx12::DX12Surface*>(m_taskData.m_hdlOutput->surface);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv;
    rtv = currentSurface->get_currentRtvDesc();
    graphicCommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

    D3D12_VIEWPORT viewport = {};
    viewport.MaxDepth       = 1.0f;
    viewport.Width          = 1280;
    viewport.Height         = 720;
    D3D12_RECT scissorRect  = {};
    scissorRect.right       = 1280;
    scissorRect.bottom      = 720;

    graphicCommandList->RSSetViewports(1, &viewport);
    graphicCommandList->RSSetScissorRects(1, &scissorRect);

    graphicCommandList->SetPipelineState(m_pso.Get());
    graphicCommandList->SetGraphicsRootSignature(m_rootSignature.Get());

    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;

    graphicCommandList->IASetPrimitiveTopology(
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    DataMeshBuffer meshBuffer = *m_taskData.m_pMeshBuffer;

    auto& buffer = m_buffers[(m_i) % dx12::DX12Surface::scm_numFrames];
    record_bind(buffer, graphicCommandList);
    graphicCommandList->DrawIndexedInstanced(meshBuffer.m_indices.size(), 1, 0,
                                             0, 0);

    dx12::DX12Context::gs_dx12Contexte->get_commandQueue().execute_commandList(
        graphicCommandList.Get());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12Task2dRender::prepare()
{
    DataMeshBuffer meshBuffer = *m_taskData.m_pMeshBuffer;

    auto& buffer = m_buffers[(++m_i) % dx12::DX12Surface::scm_numFrames];
    render::upload_data(buffer, meshBuffer.m_vertices, meshBuffer.m_indices);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Task* TaskData2dRender::getNew_dx12Implementation(TaskData* a_data)
{
    return new Dx12Task2dRender(static_cast<TaskData2dRender*>(a_data));
}
#endif  // M_DX12_RENDERER

#ifdef M_VULKAN_RENDERER
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
VulkanTask2dRender::VulkanTask2dRender(TaskData2dRender* a_data)
    : Task2dRender(a_data)
{
    for (auto& buffer : m_buffers) { init_buffer(buffer); }

    m_vertShaderModule =
        vulkan::VulkanContext::create_shaderModule("data/squareShader.vs.spv");
    m_fragShaderModule =
        vulkan::VulkanContext::create_shaderModule("data/squareShader.fs.spv");

    create_renderPassAndPipeline(1280, 720);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
VulkanTask2dRender::~VulkanTask2dRender()
{
    for (auto& buffer : m_buffers) { destroy_buffer(buffer); }

    VkDevice device = vulkan::VulkanContext::get_logDevice();

    vkDestroyPipeline(device, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
    vkDestroyRenderPass(device, m_renderPass, nullptr);

    vkDestroyShaderModule(device, m_vertShaderModule, nullptr);
    vkDestroyShaderModule(device, m_fragShaderModule, nullptr);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanTask2dRender::create_renderPassAndPipeline(U32 a_width, U32 a_height)
{
    VkDevice device = vulkan::VulkanContext::get_logDevice();

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = m_vertShaderModule;
    vertShaderStageInfo.pName  = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = m_fragShaderModule;
    fragShaderStageInfo.pName  = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding   = 0;
    bindingDescription.stride    = sizeof(BasicVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
    attributeDescriptions[0].binding                                       = 0;
    attributeDescriptions[0].location                                      = 0;
    attributeDescriptions[0].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[0].offset   = offsetof(BasicVertex, position);
    attributeDescriptions[1].binding  = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[1].offset   = offsetof(BasicVertex, color);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions =
        &bindingDescription;  // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 2;
    vertexInputInfo.pVertexAttributeDescriptions =
        attributeDescriptions.data();  // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    inputAssembly.primitiveRestartEnable = VK_TRUE;

    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = Float(a_width);
    viewport.height   = Float(a_height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {a_width, a_height};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &viewport;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;  // Optional
    rasterizer.depthBiasClamp          = 0.0f;  // Optional
    rasterizer.depthBiasSlopeFactor    = 0.0f;  // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;      // Optional
    multisampling.pSampleMask           = nullptr;   // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE;  // Optional
    multisampling.alphaToOneEnable      = VK_FALSE;  // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable         = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ZERO;                                        // Optional
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;      // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    colorBlendAttachment.dstAlphaBlendFactor =
        VK_BLEND_FACTOR_ZERO;                             // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;  // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable     = VK_FALSE;
    colorBlending.logicOp           = VK_LOGIC_OP_COPY;  // Optional
    colorBlending.attachmentCount   = 1;
    colorBlending.pAttachments      = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;  // Optional
    colorBlending.blendConstants[1] = 0.0f;  // Optional
    colorBlending.blendConstants[2] = 0.0f;  // Optional
    colorBlending.blendConstants[3] = 0.0f;  // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = 0;        // Optional
    pipelineLayoutInfo.pSetLayouts            = nullptr;  // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;        // Optional
    pipelineLayoutInfo.pPushConstantRanges    = nullptr;  // Optional

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                               &m_pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkAttachmentDescription attachment = {};
    attachment.format  = vulkan::VulkanSurface::scm_selectedSwapChainFormat;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout =
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // COLOR_ATTACHMENT_OPTIMAL;
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
    vkCreateRenderPass(device, &info, nullptr, &m_renderPass);

    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                      VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates    = dynamicStates;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages    = shaderStages;
    pipelineInfo.pVertexInputState   = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pDepthStencilState  = nullptr;  // Optional
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = &dynamicState;  // Optional
    pipelineInfo.layout              = m_pipelineLayout;
    pipelineInfo.renderPass          = m_renderPass;
    pipelineInfo.subpass             = 0;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;  // Optional
    pipelineInfo.basePipelineIndex   = -1;              // Optional

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
                                  nullptr, &m_graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanTask2dRender::prepare()
{
    DataMeshBuffer meshBuffer = *m_taskData.m_pMeshBuffer;

    auto& buffer =
        m_buffers[(++m_i) % vulkan::VulkanSurface::scm_numFrames];
    render::upload_data(buffer, meshBuffer.m_vertices, meshBuffer.m_indices);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanTask2dRender::execute() const
{
    DataMeshBuffer meshBuffer = *m_taskData.m_pMeshBuffer;

    auto currentSurface = static_cast<vulkan::VulkanSurface*>(
        m_taskData.m_hdlOutput->surface);
    auto framebuffer   = currentSurface->get_currentFramebuffer();
    auto commandBuffer = currentSurface->get_currentCommandBuffer();

    Double width  = currentSurface->get_width();
    Double height = currentSurface->get_height();

    {
        VkRenderPassBeginInfo info = {};
        info.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass  = m_renderPass;
        info.framebuffer = framebuffer;
        info.renderArea.extent.width  = width;
        info.renderArea.extent.height = height;
        VkClearValue clearValues[1]   = {};
        clearValues[0].color          = {0.4f, 0.6f, 0.9f, 1.0f};
        info.clearValueCount          = 1;
        info.pClearValues             = clearValues;
        vkCmdBeginRenderPass(commandBuffer, &info,
                             VK_SUBPASS_CONTENTS_INLINE);
    }

    if (meshBuffer.m_indices.size() > 0)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_graphicsPipeline);

        auto& buffer =
            m_buffers[(m_i) % vulkan::VulkanSurface::scm_numFrames];
        VkBuffer     vertexBuffers[] = {buffer.vertexBuffer};
        VkDeviceSize offsets[]       = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, buffer.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT16);

        VkViewport viewport{};
        viewport.x        = 0.0f;
        viewport.y        = 0.0f;
        viewport.width    = width;
        viewport.height   = height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {UInt(width), UInt(height)};

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdDrawIndexed(commandBuffer, meshBuffer.m_indices.size(), 1, 0,
                         0, 0);
    }

    // Submit command buffer
    vkCmdEndRenderPass(commandBuffer);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Task* TaskData2dRender::getNew_vulkanImplementation(TaskData* a_data)
{
    return new VulkanTask2dRender(static_cast<TaskData2dRender*>(a_data));
}
#endif  // M_VULKAN_RENDERER

}  // namespace m::render