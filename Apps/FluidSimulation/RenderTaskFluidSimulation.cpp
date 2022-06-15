#include "RenderTaskFluidSimulation.hpp"

using namespace m;
using namespace m::render;



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
TaskFluidSimulation::TaskFluidSimulation(TaskDataFluidSimulation* a_data)
{
    mSoftAssert(a_data != nullptr);
    m_taskData = *a_data;
}

#ifdef M_DX12_RENDERER
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Dx12TaskFluidSimulation::Dx12TaskFluidSimulation(
    TaskDataFluidSimulation* a_data)
    : TaskFluidSimulation(a_data)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

    dx12::ComPtr<ID3DBlob> vs =
        dx12::compile_shader("data/displayFluid.hlsl", "vs_main", "vs_6_0");
    dx12::ComPtr<ID3DBlob> ps =
        dx12::compile_shader("data/displayFluid.hlsl", "ps_main", "ps_6_0");

    pipelineDesc.InputLayout.pInputElementDescs = nullptr;
    pipelineDesc.InputLayout.NumElements        = 0;
    pipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;

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
    vRootParameters.resize(2);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vSamplerRanges;
    vSamplerRanges.resize(1);
    vSamplerRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0);
    std::vector<CD3DX12_DESCRIPTOR_RANGE> vTextureRanges;
    vTextureRanges.resize(1);
    vTextureRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    vRootParameters[0].InitAsDescriptorTable(1, vSamplerRanges.data(),
                                             D3D12_SHADER_VISIBILITY_PIXEL);
    vRootParameters[1].InitAsDescriptorTable(1, vTextureRanges.data(),
                                             D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
    descRootSignature.Init(vRootParameters.size(), vRootParameters.data(), 0,
                           nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

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
    ID3D12Device2* pDevice = dx12::DX12Context::gs_dx12Contexte->m_device.Get();

    dx12::check_MicrosoftHRESULT(device->CreateRootSignature(
        0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature)));

    pipelineDesc.pRootSignature = m_rootSignature.Get();

    dx12::check_MicrosoftHRESULT(device->CreateGraphicsPipelineState(
        &pipelineDesc, IID_PPV_ARGS(&m_pso)));

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

    for (int i = 0; i < dx12::DX12Surface::scm_numFrames; ++i)
    {
        m_GPUDescHdlTexture[i] = CD3DX12_GPU_DESCRIPTOR_HANDLE(
            m_pSrvHeap->GetGPUDescriptorHandleForHeapStart(), i,
            m_incrementSizeSrv);
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE const hldCPUSampler(
        m_pSamplerHeap->GetCPUDescriptorHandleForHeapStart(), 0,
        m_incrementSizeSampler);

    pDevice->CreateSampler(&descSampler, hldCPUSampler);

    m_GPUDescHdlSampler = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        m_pSamplerHeap->GetGPUDescriptorHandleForHeapStart(), 0,
        m_incrementSizeSampler);

    // Initialize Textures ----------------------------------------------------
    D3D12_RESOURCE_DESC descTexture{};
    descTexture.MipLevels          = 1;
    descTexture.Width              = s_nbCol;
    descTexture.Height             = s_nbRow;
    descTexture.Flags              = D3D12_RESOURCE_FLAG_NONE;
    descTexture.DepthOrArraySize   = 1;
    descTexture.SampleDesc.Count   = 1;
    descTexture.SampleDesc.Quality = 0;
    descTexture.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    descTexture.Format             = DXGI_FORMAT_R32G32B32A32_FLOAT;

    for (int i = 0; i < dx12::DX12Surface::scm_numFrames; ++i)
    {
        m_pTextureResources.emplace_back();

        auto oHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        hr                   = pDevice->CreateCommittedResource(
                              &oHeapProperties, D3D12_HEAP_FLAG_NONE, &descTexture,
                              D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr,
            IID_PPV_ARGS(m_pTextureResources.back().GetAddressOf()));
        if (hr != S_OK)
        {
            mLog_error("Fail to create resource for texture");
        }

        m_pUploadResources.emplace_back();

        const UINT64 uploadBufferSize =
            GetRequiredIntermediateSize(m_pTextureResources.back().Get(), 0, 1);

        oHeapProperties    = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto oResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
        hr                 = pDevice->CreateCommittedResource(
                            &oHeapProperties, D3D12_HEAP_FLAG_NONE, &oResourceDesc,
                            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(m_pUploadResources.back().GetAddressOf()));

        if (hr != S_OK)
        {
            mLog_error("Fail to create upload resource for texture");
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC descShaderResourceView = {};
        descShaderResourceView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        descShaderResourceView.Shader4ComponentMapping =
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        descShaderResourceView.Format              = descTexture.Format;
        descShaderResourceView.Texture2D.MipLevels = descTexture.MipLevels;
        descShaderResourceView.Texture2D.MostDetailedMip     = 0;
        descShaderResourceView.Texture2D.ResourceMinLODClamp = 0.0f;

        CD3DX12_CPU_DESCRIPTOR_HANDLE const hdlCPUSrv(
            m_pSrvHeap->GetCPUDescriptorHandleForHeapStart(),
            i, m_incrementSizeSrv);

        pDevice->CreateShaderResourceView(m_pTextureResources.back().Get(),
                                          &descShaderResourceView, hdlCPUSrv);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::prepare()
{
    m_i = (m_i + 1) % dx12::DX12Surface::scm_numFrames;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::execute() const
{
    dx12::ComPtr<ID3D12GraphicsCommandList2> graphicCommandList =
        dx12::DX12Context::gs_dx12Contexte->get_commandQueue()
            .get_commandList();

    ID3D12Resource* pTextureResource = m_pTextureResources[m_i].Get();
    ID3D12Resource* pUploadResource  = m_pUploadResources[m_i].Get();

    auto oResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        pTextureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_COPY_DEST);
    graphicCommandList->ResourceBarrier(1, &oResourceBarrier);

    std::vector<D3D12_SUBRESOURCE_DATA> vSubresources(1);
    // mip level 0
    size_t stNumBytes;
    size_t stRowBytes;
    size_t stNumRows;
    m::dx12::get_dxgiSurfaceInfo(size_t(s_nbCol), size_t(s_nbRow),
                                 DXGI_FORMAT_R32G32B32A32_FLOAT, &stNumBytes,
                                 &stRowBytes, &stNumRows);
    D3D12_SUBRESOURCE_DATA& oTextureData = vSubresources[0];
    oTextureData.pData                   = m_taskData.m_pPixelData->data();
    oTextureData.SlicePitch              = stNumBytes;
    oTextureData.RowPitch                = stRowBytes;

    UpdateSubresources(graphicCommandList.Get(), pTextureResource,
                       pUploadResource, 0, 0, 1, vSubresources.data());

    oResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        pTextureResource, D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    graphicCommandList->ResourceBarrier(1, &oResourceBarrier);

    ID3D12DescriptorHeap* const aHeaps[2] = {m_pSrvHeap.Get(),
                                             m_pSamplerHeap.Get()};

    graphicCommandList->SetDescriptorHeaps(2, aHeaps);

    auto currentSurface =
        static_cast<dx12::DX12Surface*>(m_taskData.m_hdlOutput->surface);

    mInt screenWidth  = 600;
    mInt screenHeight = 600;

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
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    graphicCommandList->SetGraphicsRootDescriptorTable(0, m_GPUDescHdlSampler);
    graphicCommandList->SetGraphicsRootDescriptorTable(
        1, m_GPUDescHdlTexture[m_i]);

    graphicCommandList->DrawInstanced(3, 1, 0, 0);

    dx12::DX12Context::gs_dx12Contexte->get_commandQueue().execute_commandList(
        graphicCommandList.Get());
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Task* TaskDataFluidSimulation::getNew_dx12Implementation(TaskData* a_data)
{
    return new Dx12TaskFluidSimulation(
        static_cast<TaskDataFluidSimulation*>(a_data));
}

#endif  // M_DX12_RENDERER

#ifdef M_VULKAN_RENDERER
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Task* TaskDataFluidSimulation::getNew_vulkanImplementation(TaskData* a_data)
{
    return nullptr;
}
#endif  // M_VULKAN_RENDERER