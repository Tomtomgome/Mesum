#include <RenderTask2DRender.hpp>
#include <Kernel/Image.hpp>
#include <array>

namespace m::render
{
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Task2dRender::Task2dRender(TaskData2dRender* a_data)
{
    mSoftAssert(a_data != nullptr);
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
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
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

    std::vector<CD3DX12_ROOT_PARAMETER> vRootParameters;
    vRootParameters.resize(4);
    vRootParameters[0].InitAsConstantBufferView(0, 0,
                                                D3D12_SHADER_VISIBILITY_VERTEX);
    vRootParameters[1].InitAsConstantBufferView(1, 0,
                                                D3D12_SHADER_VISIBILITY_PIXEL);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vSamplerRanges;
    vSamplerRanges.resize(1);
    vSamplerRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0);
    std::vector<CD3DX12_DESCRIPTOR_RANGE> vTextureRanges;
    vTextureRanges.resize(1);
    vTextureRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 1);

    vRootParameters[2].InitAsDescriptorTable(1, vSamplerRanges.data(),
                                             D3D12_SHADER_VISIBILITY_PIXEL);
    vRootParameters[3].InitAsDescriptorTable(1, vTextureRanges.data(),
                                             D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
    descRootSignature.Init(
        vRootParameters.size(), vRootParameters.data(), 0, nullptr,
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
            mLog_info((char*)errorBlob->GetBufferPointer());
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

    // LOAD THE CONSTANT BUFFERS ----------------------------------------------
    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resourceDesc   = CD3DX12_RESOURCE_DESC::Buffer(sizeof(math::mMat4x4));
    dx12::check_MicrosoftHRESULT(device->CreateCommittedResource(
        &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(m_pCbMatrices.GetAddressOf())));
    m_pCbMatrices->SetName(L"2D Matricies buffer");

    CD3DX12_RANGE readRange(0, 0);
    dx12::check_MicrosoftHRESULT(
        m_pCbMatrices->Map(0, &readRange, &m_pCbMatricesData));

    resourceDesc =
        CD3DX12_RESOURCE_DESC::Buffer(sm_nbMaxMaterial * sm_minimalCBSize);
    dx12::check_MicrosoftHRESULT(device->CreateCommittedResource(
        &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(m_pCbMaterial.GetAddressOf())));
    m_pCbMaterial->SetName(L"2D Material Buffer");

    dx12::check_MicrosoftHRESULT(
        m_pCbMaterial->Map(0, &readRange, &m_pCbMaterialData));

    ID3D12Device2* pDevice = dx12::DX12Context::gs_dx12Contexte->m_device.Get();

    // Sampler
    D3D12_SAMPLER_DESC descSampler{};
    descSampler.Filter         = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    descSampler.MinLOD         = 0;
    descSampler.MaxLOD         = 0;
    descSampler.MipLODBias     = 0.0f;
    descSampler.MaxAnisotropy  = 1;
    descSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

    D3D12_FILTER_TYPE eDx12FilterMinMag = D3D12_FILTER_TYPE_LINEAR;
    D3D12_FILTER_TYPE eDx12FilterMip    = D3D12_FILTER_TYPE_POINT;

    D3D12_FILTER_REDUCTION_TYPE eDx12FilterReduction =
        D3D12_FILTER_REDUCTION_TYPE_STANDARD;
    D3D12_TEXTURE_ADDRESS_MODE eDx12AddressMode =
        D3D12_TEXTURE_ADDRESS_MODE_WRAP;

    descSampler.Filter =
        D3D12_ENCODE_BASIC_FILTER(eDx12FilterMinMag, eDx12FilterMinMag,
                                  eDx12FilterMip, eDx12FilterReduction);
    descSampler.AddressU = eDx12AddressMode;
    descSampler.AddressV = eDx12AddressMode;
    descSampler.AddressW = eDx12AddressMode;

    // HEAPS ------------------------------------------------------------------
    m_incrementSizeSrv = pDevice->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_DESCRIPTOR_HEAP_DESC sSrvHeapDesc = {};
    sSrvHeapDesc.NumDescriptors             = sm_sizeSrvHeap;
    sSrvHeapDesc.Type     = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    sSrvHeapDesc.NodeMask = 0;
    sSrvHeapDesc.Flags    = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    HRESULT hr = pDevice->CreateDescriptorHeap(
        &sSrvHeapDesc, IID_PPV_ARGS(m_pSrvHeap.GetAddressOf()));
    mAssert(hr == S_OK);
    m_pSrvHeap.Get()->SetName(L"Texture SRV Heap");

    m_incrementSizeSampler = pDevice->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    D3D12_DESCRIPTOR_HEAP_DESC sSamplerHeapDesc = {};
    sSamplerHeapDesc.NumDescriptors             = sm_sizeSamplerHeap;
    sSamplerHeapDesc.Type     = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    sSamplerHeapDesc.NodeMask = 0;
    sSamplerHeapDesc.Flags    = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    hr = pDevice->CreateDescriptorHeap(
        &sSamplerHeapDesc, IID_PPV_ARGS(m_pSamplerHeap.GetAddressOf()));
    mAssert(hr == S_OK);
    m_pSamplerHeap.Get()->SetName(L"Texture Sampler Heap");

    // Descriptors ------------------------------------------------------------
    m_GPUDescHdlTexture = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        m_pSrvHeap->GetGPUDescriptorHandleForHeapStart(), 0,
        m_incrementSizeSrv);

    CD3DX12_CPU_DESCRIPTOR_HANDLE const hldCPUSampler(
        m_pSamplerHeap->GetCPUDescriptorHandleForHeapStart(), 0,
        m_incrementSizeSampler);

    pDevice->CreateSampler(&descSampler, hldCPUSampler);

    m_GPUDescHdlSampler = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        m_pSamplerHeap->GetGPUDescriptorHandleForHeapStart(), 0,
        m_incrementSizeSampler);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mBool Dx12Task2dRender::add_texture(resource::mRequestImage const& a_request)
{
    auto [msg, image] = resource::load_image(a_request);

    if (mNotSuccess(msg))
    {
        return false;
    }

    D3D12_RESOURCE_DESC descTexture{};
    descTexture.MipLevels          = 1;
    descTexture.Width              = image.width;
    descTexture.Height             = image.height;
    descTexture.Flags              = D3D12_RESOURCE_FLAG_NONE;
    descTexture.DepthOrArraySize   = 1;
    descTexture.SampleDesc.Count   = 1;
    descTexture.SampleDesc.Quality = 0;
    descTexture.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    descTexture.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;

    ID3D12Device2* pDevice = dx12::DX12Context::gs_dx12Contexte->m_device.Get();

    m_pTextureResources.emplace_back();
    ID3D12Resource* pTextureResource = m_pTextureResources.back().Get();

    auto    oHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    HRESULT hr              = pDevice->CreateCommittedResource(
                     &oHeapProperties, D3D12_HEAP_FLAG_NONE, &descTexture,
                     D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                     IID_PPV_ARGS(&pTextureResource));
    if (hr != S_OK)
    {
        mLog_error("Fail to create resource for texture");
        return false;
    }

    pTextureResource->SetName(L"defaultName");

    ID3D12Resource* pUploadTextureResource;
    const UINT      subresourceCount =
        descTexture.DepthOrArraySize * descTexture.MipLevels;

    // CREATE UPLOAD (CPU SIDE) RESOURCE
    const UINT64 uploadBufferSize =
        GetRequiredIntermediateSize(pTextureResource, 0, subresourceCount);

    oHeapProperties    = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto oResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    hr                 = pDevice->CreateCommittedResource(
                        &oHeapProperties, D3D12_HEAP_FLAG_NONE, &oResourceDesc,
                        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                        IID_PPV_ARGS(&pUploadTextureResource));
    if (hr != S_OK)
    {
        pTextureResource->Release();
        mLog_error("Fail to create upload resource for texture");
        return false;
    }

    std::vector<D3D12_SUBRESOURCE_DATA> vSubresources(descTexture.MipLevels);
    // mip level 0
    size_t stNumBytes;
    size_t stRowBytes;
    size_t stNumRows;
    dx12::get_dxgiSurfaceInfo(size_t(descTexture.Width), size_t(descTexture.Height),
                        descTexture.Format, &stNumBytes, &stRowBytes,
                        &stNumRows);
    D3D12_SUBRESOURCE_DATA& oTextureData = vSubresources[0];
    oTextureData.pData                   = image.data.data();
    oTextureData.SlicePitch              = stNumBytes;
    oTextureData.RowPitch                = stRowBytes;

    dx12::ComPtr<ID3D12GraphicsCommandList2> pUploadCommandList =
        dx12::DX12Context::gs_dx12Contexte->get_commandQueue()
            .get_commandList();

    UpdateSubresources(pUploadCommandList.Get(), pTextureResource,
                       pUploadTextureResource, 0, 0, subresourceCount,
                       vSubresources.data());

    D3D12_RESOURCE_STATES eAfterState =
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    auto oResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        pTextureResource, D3D12_RESOURCE_STATE_COPY_DEST, eAfterState);
    pUploadCommandList->ResourceBarrier(1, &oResourceBarrier);

    dx12::DX12Context::gs_dx12Contexte->get_commandQueue().execute_commandList(
        pUploadCommandList.Get());

    D3D12_SHADER_RESOURCE_VIEW_DESC descShaderResourceView = {};
    descShaderResourceView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    descShaderResourceView.Shader4ComponentMapping =
        D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    descShaderResourceView.Format                    = descTexture.Format;
    descShaderResourceView.Texture2D.MipLevels       = descTexture.MipLevels;
    descShaderResourceView.Texture2D.MostDetailedMip = 0;
    descShaderResourceView.Texture2D.ResourceMinLODClamp = 0.0f;

    CD3DX12_CPU_DESCRIPTOR_HANDLE const hdlCPUSrv(
        m_pSrvHeap->GetCPUDescriptorHandleForHeapStart(),
        m_pTextureResources.size() - 1, m_incrementSizeSrv);

    pDevice->CreateShaderResourceView(pTextureResource, &descShaderResourceView,
                                      hdlCPUSrv);

    return true;
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
void Dx12Task2dRender::execute() const
{
    if (m_taskData.m_pRanges->size() == 0)
    {
        return;
    }
    dx12::ComPtr<ID3D12GraphicsCommandList2> graphicCommandList =
        dx12::DX12Context::gs_dx12Contexte->get_commandQueue()
            .get_commandList();

    ID3D12DescriptorHeap* const aHeaps[2] = {m_pSrvHeap.Get(),
                                             m_pSamplerHeap.Get()};

    graphicCommandList->SetDescriptorHeaps(2, aHeaps);

    auto currentSurface =
        static_cast<dx12::DX12Surface*>(m_taskData.m_hdlOutput->surface);

    mInt screenWidth  = 1280;
    mInt screenHeight = 720;

    D3D12_CPU_DESCRIPTOR_HANDLE rtv;
    rtv = currentSurface->get_currentRtvDesc();
    graphicCommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
    D3D12_VIEWPORT viewport = {};
    viewport.MaxDepth       = 1.0f;
    viewport.Width          = screenWidth;
    viewport.Height         = screenHeight;
    D3D12_RECT scissorRect  = {};
    scissorRect.right       = screenWidth;
    scissorRect.bottom      = screenHeight;

    graphicCommandList->RSSetViewports(1, &viewport);
    graphicCommandList->RSSetScissorRects(1, &scissorRect);

    graphicCommandList->SetPipelineState(m_pso.Get());
    graphicCommandList->SetGraphicsRootSignature(m_rootSignature.Get());

    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;

    graphicCommandList->IASetPrimitiveTopology(
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    math::mMat4x4& mvpMatrix = *((math::mMat4x4*)(m_pCbMatricesData));

    mvpMatrix = *m_taskData.m_pMatrix;
    mvpMatrix.transpose();

    graphicCommandList->SetGraphicsRootConstantBufferView(
        0, m_pCbMatrices->GetGPUVirtualAddress());

    graphicCommandList->SetGraphicsRootDescriptorTable(2, m_GPUDescHdlSampler);
    graphicCommandList->SetGraphicsRootDescriptorTable(3, m_GPUDescHdlTexture);

    DataMeshBuffer meshBuffer = *m_taskData.m_pMeshBuffer;
    auto&          buffer = m_buffers[(m_i) % dx12::DX12Surface::scm_numFrames];
    record_bind(buffer, graphicCommandList);

    for (mUInt i = 0; i < m_taskData.m_pRanges->size(); i++)
    {
        auto& range = (*m_taskData.m_pRanges)[i];
        // upload constant buffer
        mInt& materialID =
            *((mInt*)((mU8*)(m_pCbMaterialData) + i * sm_minimalCBSize));
        materialID = range.materialID;

        graphicCommandList->SetGraphicsRootConstantBufferView(
            1, m_pCbMaterial->GetGPUVirtualAddress() + i * sm_minimalCBSize);

        // draw
        graphicCommandList->DrawIndexedInstanced(
            range.indexCount, 1, range.indexStartLocation, 0, 0);
    }

    dx12::DX12Context::gs_dx12Contexte->get_commandQueue().execute_commandList(
        graphicCommandList.Get());
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

    VkDevice device = vulkan::VulkanContext::get_logDevice();

    // Sampler ----------------------------------------------------------------
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter               = VK_FILTER_LINEAR;
    samplerInfo.minFilter               = VK_FILTER_LINEAR;
    samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable        = VK_FALSE;
    samplerInfo.maxAnisotropy           = 1.0f;
    samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias              = 0.0f;
    samplerInfo.minLod                  = 0.0f;
    samplerInfo.maxLod                  = 0.0f;

    vulkan::check_vkResult(
        vkCreateSampler(device, &samplerInfo, nullptr, &m_textureSampler));

    // Bindings and layout ----------------------------------------------------
    std::array<VkDescriptorSetLayoutBinding, 2> bindings;
    bindings[0]                 = {};
    bindings[0].binding         = 0;
    bindings[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[1]                 = {};
    bindings[1].binding         = 1;
    bindings[1].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo createInfoLayout{};
    createInfoLayout.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfoLayout.bindingCount = bindings.size();
    createInfoLayout.pBindings    = bindings.data();

    vulkan::check_vkResult(vkCreateDescriptorSetLayout(
        device, &createInfoLayout, nullptr, &m_cbDescriptorLayout));

    // Bindless
    VkDescriptorBindingFlags bindingFlagsBindless =
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;

    VkDescriptorSetLayoutBinding binding{};
    binding.binding         = 0;
    binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = sm_sizeDescriptorPool;
    binding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;

    createInfoLayout.bindingCount = 1;
    createInfoLayout.pBindings    = &binding;
    createInfoLayout.flags =
        VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extendedCreateInfoLayout{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
        nullptr};
    extendedCreateInfoLayout.bindingCount  = 1;
    extendedCreateInfoLayout.pBindingFlags = &bindingFlagsBindless;

    createInfoLayout.pNext = &extendedCreateInfoLayout;
    vulkan::check_vkResult(
        vkCreateDescriptorSetLayout(device, &createInfoLayout, nullptr,
                                    &m_bindlessTextureDescriptorLayout));

    // Create pipeline --------------------------------------------------------
    vulkan::VulkanSurface* pSurface =
        ((vulkan::VulkanSurface*)(a_data->m_hdlOutput->surface));
    mU32 width  = pSurface->get_width();
    mU32 height = pSurface->get_height();
    create_renderPassAndPipeline(width, height);

    // Allocate Constant buffers ----------------------------------------------
    VkDeviceSize bufferSize = sizeof(math::mMat4x4);
    VkDeviceSize materialBufferSize =
        sm_minimalCBSize * sm_nbMaxMaterial * sizeof(mInt);

    for (size_t i = 0; i < vulkan::VulkanSurface::scm_numFrames; i++)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size        = bufferSize;
        bufferInfo.usage       = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vulkan::check_vkResult(
            vkCreateBuffer(device, &bufferInfo, nullptr, &m_cbMatrices[i]));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, m_cbMatrices[i],
                                      &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize  = memRequirements.size;
        allocInfo.memoryTypeIndex = vulkan::VulkanContext::get_memoryTypeIndex(
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vulkan::check_vkResult(vkAllocateMemory(device, &allocInfo, nullptr,
                                                &m_cbMatricesMemory[i]));

        vkBindBufferMemory(device, m_cbMatrices[i], m_cbMatricesMemory[i], 0);

        // Materials buffer
        bufferInfo.size = materialBufferSize;
        ;
        bufferInfo.usage       = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vulkan::check_vkResult(
            vkCreateBuffer(device, &bufferInfo, nullptr, &m_cbMaterials[i]));

        vkGetBufferMemoryRequirements(device, m_cbMaterials[i],
                                      &memRequirements);

        allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize  = memRequirements.size;
        allocInfo.memoryTypeIndex = vulkan::VulkanContext::get_memoryTypeIndex(
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vulkan::check_vkResult(vkAllocateMemory(device, &allocInfo, nullptr,
                                                &m_cbMaterialsMemory[i]));

        vkBindBufferMemory(device, m_cbMaterials[i], m_cbMaterialsMemory[i], 0);
    }

    // Descriptror pools ------------------------------------------------------
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         vulkan::VulkanSurface::scm_numFrames},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
         sm_nbMaxMaterial * vulkan::VulkanSurface::scm_numFrames}};

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes    = poolSizes.data();
    poolInfo.maxSets       = vulkan::VulkanSurface::scm_numFrames;

    vulkan::check_vkResult(
        vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool));

    // Bindless
    std::array<VkDescriptorPoolSize, 1> poolSizesBindless = {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, sm_sizeDescriptorPool}};
    poolInfo.poolSizeCount = poolSizesBindless.size();
    poolInfo.pPoolSizes    = poolSizesBindless.data();
    poolInfo.maxSets       = sm_sizeDescriptorPool * poolSizesBindless.size();
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
    vulkan::check_vkResult(vkCreateDescriptorPool(device, &poolInfo, nullptr,
                                                  &m_textureDescriptorPool));

    // Descriptors allocation -------------------------------------------------
    VkDescriptorSetLayout layouts[vulkan::VulkanSurface::scm_numFrames] = {
        m_cbDescriptorLayout, m_cbDescriptorLayout, m_cbDescriptorLayout};

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = m_descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount =
        vulkan::VulkanSurface::scm_numFrames;
    descriptorSetAllocInfo.pSetLayouts = layouts;

    vulkan::check_vkResult(
        vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, m_cbSets));

    // Bindless
    descriptorSetAllocInfo.descriptorPool     = m_textureDescriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = &m_bindlessTextureDescriptorLayout;

    VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT};
    mU32 maxBinding              = sm_sizeDescriptorPool - 1;
    countInfo.descriptorSetCount = 1;
    // This number is the max allocatable count
    countInfo.pDescriptorCounts  = &maxBinding;
    descriptorSetAllocInfo.pNext = &countInfo;

    vulkan::check_vkResult(vkAllocateDescriptorSets(
        device, &descriptorSetAllocInfo, &m_bindlessTextureDescriptorSet));

    // Descriptor mapping -----------------------------------------------------
    for (size_t i = 0; i < vulkan::VulkanSurface::scm_numFrames; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_cbMatrices[i];
        bufferInfo.offset = 0;
        bufferInfo.range  = sizeof(math::mMat4x4);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet     = m_cbSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement  = 0;
        descriptorWrite.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount  = 1;
        descriptorWrite.pBufferInfo      = &bufferInfo;
        descriptorWrite.pImageInfo       = nullptr;
        descriptorWrite.pTexelBufferView = nullptr;

        VkDescriptorBufferInfo materialBufferInfo{};
        materialBufferInfo.buffer = m_cbMaterials[i];
        materialBufferInfo.offset = 0;
        materialBufferInfo.range  = sizeof(mInt);

        VkWriteDescriptorSet materialDescriptorWrite{};
        materialDescriptorWrite.sType  = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        materialDescriptorWrite.dstSet = m_cbSets[i];
        materialDescriptorWrite.dstBinding      = 1;
        materialDescriptorWrite.dstArrayElement = 0;
        materialDescriptorWrite.descriptorType =
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        materialDescriptorWrite.descriptorCount  = 1;
        materialDescriptorWrite.pBufferInfo      = &materialBufferInfo;
        materialDescriptorWrite.pImageInfo       = nullptr;
        materialDescriptorWrite.pTexelBufferView = nullptr;

        VkWriteDescriptorSet setWrites[] = {descriptorWrite,
                                            materialDescriptorWrite};

        vkUpdateDescriptorSets(device, 2, setWrites, 0, nullptr);
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
VulkanTask2dRender::~VulkanTask2dRender()
{
    for (auto& buffer : m_buffers) { destroy_buffer(buffer); }

    VkDevice device = vulkan::VulkanContext::get_logDevice();

    for (size_t i = 0; i < vulkan::VulkanSurface::scm_numFrames; i++)
    {
        vkDestroyBuffer(device, m_cbMatrices[i], nullptr);
        vkFreeMemory(device, m_cbMatricesMemory[i], nullptr);

        vkDestroyBuffer(device, m_cbMaterials[i], nullptr);
        vkFreeMemory(device, m_cbMaterialsMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
    vkDestroyDescriptorPool(device, m_textureDescriptorPool, nullptr);

    vkDestroyPipeline(device, m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
    vkDestroyRenderPass(device, m_renderPass, nullptr);

    vkDestroyDescriptorSetLayout(device, m_cbDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, m_bindlessTextureDescriptorLayout,
                                 nullptr);

    vkDestroySampler(device, m_textureSampler, nullptr);
    for (mUInt i = 0; i < m_pTextureImages.size(); ++i)
    {
        vkDestroyImageView(device, m_imageViews[i], nullptr);
        vkDestroyImage(device, m_pTextureImages[i], nullptr);
        vkFreeMemory(device, m_pTextureMemory[i], nullptr);
    }

    vkDestroyShaderModule(device, m_vertShaderModule, nullptr);
    vkDestroyShaderModule(device, m_fragShaderModule, nullptr);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mBool VulkanTask2dRender::add_texture(resource::mRequestImage const& a_request)
{
    auto [msg, image] = resource::load_image(a_request);

    if (mNotSuccess(msg))
    {
        return false;
    }

    mUInt imageSize = image.width * image.height * 4;

    VkDevice               device = vulkan::VulkanContext::get_logDevice();
    vulkan::VulkanContext& vkContext =
        *vulkan::VulkanContext::gs_VulkanContexte;

    // Upload buffer ----------------------------------------------------------
    VkBuffer       stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size        = imageSize;
    bufferInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vulkan::check_vkResult(
        vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = memRequirements.size;
    allocInfo.memoryTypeIndex = vulkan::VulkanContext::get_memoryTypeIndex(
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &stagingBufferMemory) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

    void* pData;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &pData);
    memcpy(pData, image.data.data(), static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    // Image ------------------------------------------------------------------
    m_pTextureImages.emplace_back();
    VkImage& textureImage = m_pTextureImages.back();
    m_pTextureMemory.emplace_back();
    VkDeviceMemory& textureMemory = m_pTextureMemory.back();

    VkImageCreateInfo imageInfo{};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width  = static_cast<uint32_t>(image.width);
    imageInfo.extent.height = static_cast<uint32_t>(image.height);
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = 1;
    imageInfo.format        = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples     = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags       = 0;  // Optional

    vulkan::check_vkResult(
        vkCreateImage(device, &imageInfo, nullptr, &textureImage));

    vkGetImageMemoryRequirements(device, textureImage, &memRequirements);

    allocInfo.allocationSize  = memRequirements.size;
    allocInfo.memoryTypeIndex = vulkan::VulkanContext::get_memoryTypeIndex(
        memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vulkan::check_vkResult(
        vkAllocateMemory(device, &allocInfo, nullptr, &textureMemory));

    vkBindImageMemory(device, textureImage, textureMemory, 0);

    // Copy image content -----------------------------------------------------
    VkCommandBuffer commandBuffer = vkContext.get_singleUseCommandBuffer();

    VkImageMemoryBarrier barrier{};
    barrier.sType     = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image                           = textureImage;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
                         nullptr, 1, &barrier);
    vkContext.submit_singleUseCommandBuffer(commandBuffer);

    commandBuffer = vkContext.get_singleUseCommandBuffer();

    VkBufferImageCopy region{};
    region.bufferOffset      = 0;
    region.bufferRowLength   = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {mU32(image.width), mU32(image.height), 1};
    vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, textureImage,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    vkContext.submit_singleUseCommandBuffer(commandBuffer);

    commandBuffer = vkContext.get_singleUseCommandBuffer();

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image                           = textureImage;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                         0, nullptr, 1, &barrier);
    vkContext.submit_singleUseCommandBuffer(commandBuffer);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    // Create imageView -------------------------------------------------------
    m_imageViews.emplace_back();
    VkImageView& imageView = m_imageViews.back();

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image    = textureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format   = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;

    vulkan::check_vkResult(
        vkCreateImageView(device, &viewInfo, nullptr, &imageView));

    // Update Descriptors -----------------------------------------------------
    VkDescriptorImageInfo descriptorImageInfo{};
    descriptorImageInfo.sampler     = m_textureSampler;
    descriptorImageInfo.imageView   = imageView;
    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet bindlessDescriptorWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    bindlessDescriptorWrite.descriptorCount = 1;
    bindlessDescriptorWrite.dstArrayElement = m_imageViews.size() - 1;
    bindlessDescriptorWrite.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindlessDescriptorWrite.dstSet     = m_bindlessTextureDescriptorSet;
    bindlessDescriptorWrite.dstBinding = 0;
    bindlessDescriptorWrite.pImageInfo = &descriptorImageInfo;

    vkUpdateDescriptorSets(device, 1, &bindlessDescriptorWrite, 0, nullptr);

    return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanTask2dRender::create_renderPassAndPipeline(mU32 a_width,
                                                      mU32 a_height)
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
    bindingDescription.stride    = sizeof(mBasicVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
    attributeDescriptions[0].binding                                       = 0;
    attributeDescriptions[0].location                                      = 0;
    attributeDescriptions[0].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[0].offset   = offsetof(mBasicVertex, position);
    attributeDescriptions[1].binding  = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[1].offset   = offsetof(mBasicVertex, color);
    attributeDescriptions[2].binding  = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset   = offsetof(mBasicVertex, uv);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions =
        &bindingDescription;  // Optional
    vertexInputInfo.vertexAttributeDescriptionCount =
        attributeDescriptions.size();
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
    viewport.width    = mFloat(a_width);
    viewport.height   = mFloat(a_height);
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
    rasterizer.cullMode                = D3D12_CULL_MODE_NONE;
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
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor =
        VK_BLEND_FACTOR_SRC_ALPHA;  // Optional
    colorBlendAttachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;              // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;  // Optional
    colorBlendAttachment.srcAlphaBlendFactor =
        VK_BLEND_FACTOR_SRC_ALPHA;  // Optional
    colorBlendAttachment.dstAlphaBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;              // Optional
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

    std::array<VkDescriptorSetLayout, 2> layouts = {
        m_cbDescriptorLayout, m_bindlessTextureDescriptorLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = layouts.size();  // Optional
    pipelineLayoutInfo.pSetLayouts            = layouts.data();  // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;               // Optional
    pipelineLayoutInfo.pPushConstantRanges    = nullptr;         // Optional

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

    auto& buffer = m_buffers[(++m_i) % vulkan::VulkanSurface::scm_numFrames];
    render::upload_data(buffer, meshBuffer.m_vertices, meshBuffer.m_indices);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void VulkanTask2dRender::execute() const
{
    DataMeshBuffer meshBuffer = *m_taskData.m_pMeshBuffer;

    auto currentSurface =
        static_cast<vulkan::VulkanSurface*>(m_taskData.m_hdlOutput->surface);
    auto framebuffer   = currentSurface->get_currentFramebuffer();
    auto commandBuffer = currentSurface->get_currentCommandBuffer();

    mDouble width  = currentSurface->get_width();
    mDouble height = currentSurface->get_height();

    mInt currentImage = (m_i) % vulkan::VulkanSurface::scm_numFrames;

    {
        VkRenderPassBeginInfo info   = {};
        info.sType                   = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass              = m_renderPass;
        info.framebuffer             = framebuffer;
        info.renderArea.extent.width = width;
        info.renderArea.extent.height = height;
        VkClearValue clearValues[1]   = {};
        clearValues[0].color          = {1.0f, 1.0f, 1.0f, 0.0f};
        info.clearValueCount          = 1;
        info.pClearValues             = clearValues;
        vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    if (meshBuffer.m_indices.size() > 0)
    {
        VkDevice device = vulkan::VulkanContext::get_logDevice();

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_graphicsPipeline);

        auto&        buffer          = m_buffers[currentImage];
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
        scissor.extent = {mUInt(width), mUInt(height)};

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        void* data;
        vkMapMemory(device, m_cbMatricesMemory[currentImage], 0,
                    sizeof(math::mMat4x4), 0, &data);
        math::mMat4x4& mvpMatrix = *((math::mMat4x4*)(data));
        mvpMatrix                = *m_taskData.m_pMatrix;
        vkUnmapMemory(device, m_cbMatricesMemory[currentImage]);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_pipelineLayout, 1, 1,
                                &m_bindlessTextureDescriptorSet, 0, nullptr);

        for (mUInt i = 0; i < m_taskData.m_pRanges->size(); i++)
        {
            auto& range = (*m_taskData.m_pRanges)[i];
            // upload constant buffer

            void* materialData;
            vkMapMemory(device, m_cbMaterialsMemory[currentImage],
                        i * sm_minimalCBSize, sizeof(mInt), 0, &materialData);
            mInt& materialID = *((mInt*)(materialData));
            materialID       = range.materialID;
            vkUnmapMemory(device, m_cbMaterialsMemory[currentImage]);

            mU32 materialOffset = sm_minimalCBSize * i;

            vkCmdBindDescriptorSets(
                commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipelineLayout, 0, 1, &m_cbSets[currentImage], 1,
                &materialOffset);

            // draw
            vkCmdDrawIndexed(commandBuffer, range.indexCount, 1,
                             range.indexStartLocation, 0, 0);
        }
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