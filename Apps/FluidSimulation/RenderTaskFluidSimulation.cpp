#include "RenderTaskFluidSimulation.hpp"

using namespace m;
using namespace m::render;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mOutput<void*> upload_toGPU(
    resource::mTypedImage<math::mVec4> const& a_image,
    m::dx12::ComPtr<ID3D12Resource>&          a_pTextureResource,
    m::dx12::ComPtr<ID3D12Resource>&          a_pUploadResource,
    DXGI_FORMAT                               a_textureFormat,
    wchar_t const* a_debugString = L"default texture name")
{
    D3D12_RESOURCE_DESC descTexture{};
    descTexture.MipLevels          = 1;
    descTexture.Width              = a_image.width;
    descTexture.Height             = a_image.height;
    descTexture.Flags              = D3D12_RESOURCE_FLAG_NONE;
    descTexture.DepthOrArraySize   = 1;
    descTexture.SampleDesc.Count   = 1;
    descTexture.SampleDesc.Quality = 0;
    descTexture.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    descTexture.Format             = a_textureFormat;

    ID3D12Device2* pDevice = dx12::DX12Context::gs_dx12Contexte->m_device.Get();

    auto    oHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    HRESULT hr              = pDevice->CreateCommittedResource(
                     &oHeapProperties, D3D12_HEAP_FLAG_NONE, &descTexture,
                     D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                     IID_PPV_ARGS(a_pTextureResource.GetAddressOf()));
    if (hr != S_OK)
    {
        mLog_error("Fail to create resource for texture");
        return {ecFailure, nullptr};
    }

    a_pTextureResource->SetName(a_debugString);

    const UINT subresourceCount =
        descTexture.DepthOrArraySize * descTexture.MipLevels;

    // CREATE UPLOAD (CPU SIDE) RESOURCE
    const UINT64 uploadBufferSize =
        GetRequiredIntermediateSize(a_pTextureResource.Get(), 0, subresourceCount);

    oHeapProperties    = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto oResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    hr                 = pDevice->CreateCommittedResource(
                        &oHeapProperties, D3D12_HEAP_FLAG_NONE, &oResourceDesc,
                        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                        IID_PPV_ARGS(a_pUploadResource.GetAddressOf()));
    if (hr != S_OK)
    {
        mLog_error("Fail to create upload resource for texture");
        return {ecFailure, nullptr};
    }

    std::vector<D3D12_SUBRESOURCE_DATA> vSubresources(descTexture.MipLevels);
    // mip level 0
    size_t stNumBytes;
    size_t stRowBytes;
    size_t stNumRows;
    dx12::get_dxgiSurfaceInfo(size_t(descTexture.Width),
                              size_t(descTexture.Height), descTexture.Format,
                              &stNumBytes, &stRowBytes, &stNumRows);
    D3D12_SUBRESOURCE_DATA& oTextureData = vSubresources[0];
    oTextureData.pData                   = a_image.data.data();
    oTextureData.SlicePitch              = stNumBytes;
    oTextureData.RowPitch                = stRowBytes;

    dx12::ComPtr<ID3D12GraphicsCommandList2> pUploadCommandList =
        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .get_commandList();

    UpdateSubresources(pUploadCommandList.Get(), a_pTextureResource.Get(),
                       a_pUploadResource.Get(), 0, 0, subresourceCount,
                       vSubresources.data());

    D3D12_RESOURCE_STATES eAfterState =
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    auto oResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        a_pTextureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, eAfterState);
    pUploadCommandList->ResourceBarrier(1, &oResourceBarrier);

    dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
        .execute_commandList(pUploadCommandList.Get());

    return {ecSuccess, nullptr};
}

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
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;
    HRESULT res;

    // Sampler
    D3D12_SAMPLER_DESC descSampler{};
    descSampler.Filter         = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    descSampler.MinLOD         = 0;
    descSampler.MaxLOD         = 0;
    descSampler.MipLODBias     = 0.0f;
    descSampler.MaxAnisotropy  = 1;
    descSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

    D3D12_FILTER_TYPE eDx12FilterMinMag = D3D12_FILTER_TYPE_POINT;
    D3D12_FILTER_TYPE eDx12FilterMip    = D3D12_FILTER_TYPE_POINT;

    D3D12_FILTER_REDUCTION_TYPE eDx12FilterReduction =
        D3D12_FILTER_REDUCTION_TYPE_STANDARD;
    D3D12_TEXTURE_ADDRESS_MODE eDx12AddressMode =
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

    descSampler.Filter =
        D3D12_ENCODE_BASIC_FILTER(eDx12FilterMinMag, eDx12FilterMinMag,
                                  eDx12FilterMip, eDx12FilterReduction);
    descSampler.AddressU = eDx12AddressMode;
    descSampler.AddressV = eDx12AddressMode;
    descSampler.AddressW = eDx12AddressMode;

    // HEAPS ------------------------------------------------------------------
    m_incrementSizeSrv = device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_DESCRIPTOR_HEAP_DESC sSrvHeapDesc = {};
    sSrvHeapDesc.NumDescriptors = sm_sizeSrvHeap + sm_sizeHeapOutBuffer;
    sSrvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    sSrvHeapDesc.NodeMask       = 0;
    sSrvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    HRESULT hr = device->CreateDescriptorHeap(
        &sSrvHeapDesc, IID_PPV_ARGS(m_pSrvHeap.GetAddressOf()));
    mAssert(hr == S_OK);
    m_pSrvHeap.Get()->SetName(L"Texture SRV Heap");

    m_incrementSizeSampler = device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    D3D12_DESCRIPTOR_HEAP_DESC sSamplerHeapDesc = {};
    sSamplerHeapDesc.NumDescriptors             = sm_sizeSamplerHeap;
    sSamplerHeapDesc.Type     = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    sSamplerHeapDesc.NodeMask = 0;
    sSamplerHeapDesc.Flags    = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    hr = device->CreateDescriptorHeap(
        &sSamplerHeapDesc, IID_PPV_ARGS(m_pSamplerHeap.GetAddressOf()));
    mAssert(hr == S_OK);
    m_pSamplerHeap.Get()->SetName(L"Texture Sampler Heap");

    // Descriptors ------------------------------------------------------------
    // Descriptors for the textures
    for (int i = 0; i < dx12::DX12Surface::scm_numFrames; ++i)
    {
        m_GPUDescHdlTexture[i] = CD3DX12_GPU_DESCRIPTOR_HANDLE(
            m_pSrvHeap->GetGPUDescriptorHandleForHeapStart(), i,
            m_incrementSizeSrv);
    }
    // Descriptor for output buffer
    m_GPUDescHdlOutBuffer = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        m_pSrvHeap->GetGPUDescriptorHandleForHeapStart(),
        dx12::DX12Surface::scm_numFrames, m_incrementSizeSrv);
    // Descriptor for sampler
    CD3DX12_CPU_DESCRIPTOR_HANDLE const hldCPUSampler(
        m_pSamplerHeap->GetCPUDescriptorHandleForHeapStart(), 0,
        m_incrementSizeSampler);

    device->CreateSampler(&descSampler, hldCPUSampler);

    m_GPUDescHdlSampler = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        m_pSamplerHeap->GetGPUDescriptorHandleForHeapStart(), 0,
        m_incrementSizeSampler);

    DXGI_FORMAT simulationTextureFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
    // Initialize Textures ----------------------------------------------------
    for (int i = 0; i < dx12::DX12Surface::scm_numFrames; ++i)
    {
        m_pTextureResources.emplace_back();
        m_pUploadResources.emplace_back();

        auto [result, _] = upload_toGPU(unref_safe(m_taskData.pInitialData),
                                        m_pTextureResources.back(),
                                        m_pUploadResources.back(),
                                        simulationTextureFormat);
        mAssert(mIsSuccess(result));

        D3D12_SHADER_RESOURCE_VIEW_DESC descShaderResourceView = {};
        descShaderResourceView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        descShaderResourceView.Shader4ComponentMapping =
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        descShaderResourceView.Format              = simulationTextureFormat;
        descShaderResourceView.Texture2D.MipLevels = 1;
        descShaderResourceView.Texture2D.MostDetailedMip     = 0;
        descShaderResourceView.Texture2D.ResourceMinLODClamp = 0.0f;

        CD3DX12_CPU_DESCRIPTOR_HANDLE const hdlCPUSrv(
            m_pSrvHeap->GetCPUDescriptorHandleForHeapStart(), i,
            m_incrementSizeSrv);

        device->CreateShaderResourceView(m_pTextureResources.back().Get(),
                                         &descShaderResourceView, hdlCPUSrv);
    }

    // -------------------- Root Signatures
    // ----------------- Field arrows rendering
    setup_fluidRenderingPass();

    // ----------------- Field arrows rendering compute
    setup_arrowGenerationPass();

    // ----------------- Field arrows rendering
    setup_arrowRenderingPass();
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
        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .get_commandList();

    // ---------------- Upload the simulated texture
//    ID3D12Resource* pTextureResource = m_pTextureResources[m_i].Get();
//    ID3D12Resource* pUploadResource  = m_pUploadResources[m_i].Get();
//
//    auto oResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
//        pTextureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
//        D3D12_RESOURCE_STATE_COPY_DEST);
//    graphicCommandList->ResourceBarrier(1, &oResourceBarrier);
//
//    std::vector<D3D12_SUBRESOURCE_DATA> vSubresources(1);
//    // mip level 0
//    size_t stNumBytes;
//    size_t stRowBytes;
//    size_t stNumRows;
//    m::dx12::get_dxgiSurfaceInfo(size_t(s_nbCol), size_t(s_nbRow),
//                                 DXGI_FORMAT_R32G32B32A32_FLOAT, &stNumBytes,
//                                 &stRowBytes, &stNumRows);
//    D3D12_SUBRESOURCE_DATA& oTextureData = vSubresources[0];
//    oTextureData.pData                   = m_taskData.pPixelData->data();
//    oTextureData.SlicePitch              = stNumBytes;
//    oTextureData.RowPitch                = stRowBytes;
//
//    UpdateSubresources(graphicCommandList.Get(), pTextureResource,
//                       pUploadResource, 0, 0, 1, vSubresources.data());
//
//    oResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
//        pTextureResource, D3D12_RESOURCE_STATE_COPY_DEST,
//        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
//    graphicCommandList->ResourceBarrier(1, &oResourceBarrier);

    // ---------------- Renders the fluid
    ID3D12DescriptorHeap* const aHeaps[2] = {m_pSrvHeap.Get(),
                                             m_pSamplerHeap.Get()};

    graphicCommandList->SetDescriptorHeaps(2, aHeaps);

    auto pOutputRT =
        static_cast<dx12::mRenderTarget const*>(m_taskData.pOutputRT);

    mInt screenWidth  = 600;
    mInt screenHeight = 600;

    D3D12_CPU_DESCRIPTOR_HANDLE rtv;
    rtv = pOutputRT->rtv;
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

    graphicCommandList->SetPipelineState(m_psoFluidRendering.Get());
    graphicCommandList->SetGraphicsRootSignature(m_rsFluidRendering.Get());

    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;

    graphicCommandList->IASetPrimitiveTopology(
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    graphicCommandList->SetGraphicsRootDescriptorTable(0, m_GPUDescHdlSampler);
    graphicCommandList->SetGraphicsRootDescriptorTable(
        1, m_GPUDescHdlTexture[m_i]);

    graphicCommandList->DrawInstanced(3, 1, 0, 0);

    dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
        .execute_commandList(graphicCommandList.Get());

    // ---------------- Field arrows display
    // ---------------- Compute part
    dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
        dx12::DX12Context::gs_dx12Contexte->get_computeCommandQueue()
            .get_commandList();

    auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_pVertexBufferArrows.Get(),
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    graphicCommandList->ResourceBarrier(1, &resourceBarrier);

    graphicCommandList->SetDescriptorHeaps(2, aHeaps);

    computeCommandList->SetPipelineState(m_psoArrowGeneration.Get());
    computeCommandList->SetComputeRootSignature(m_rsArrowGeneration.Get());

    computeCommandList->SetComputeRootDescriptorTable(0, m_GPUDescHdlSampler);
    computeCommandList->SetComputeRootDescriptorTable(1,
                                                      m_GPUDescHdlTexture[m_i]);
    computeCommandList->SetComputeRootDescriptorTable(2, m_GPUDescHdlOutBuffer);

    computeCommandList->Dispatch(s_nbRow, s_nbCol, 1);

    dx12::DX12Context::gs_dx12Contexte->get_computeCommandQueue()
        .execute_commandList(computeCommandList);

    // ---------------- Render part

    dx12::ComPtr<ID3D12GraphicsCommandList2> graphicsCommandListField =
        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .get_commandList();

    resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_pVertexBufferArrows.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    graphicsCommandListField->ResourceBarrier(1, &resourceBarrier);

    graphicsCommandListField->SetDescriptorHeaps(2, aHeaps);

    graphicCommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
    graphicCommandList->RSSetViewports(1, &viewport);
    graphicCommandList->RSSetScissorRects(1, &scissorRect);

    graphicsCommandListField->SetPipelineState(m_psoArrowRendering.Get());
    graphicsCommandListField->SetGraphicsRootSignature(
        m_rsArrowRendering.Get());

    graphicCommandList->IASetPrimitiveTopology(
        D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);

    D3D12_VERTEX_BUFFER_VIEW vbv;
    memset(&vbv, 0, sizeof(D3D12_VERTEX_BUFFER_VIEW));
    vbv.BufferLocation = m_pVertexBufferArrows->GetGPUVirtualAddress();
    vbv.SizeInBytes =
        s_nbRow * s_nbCol * sm_nbVertexPerArrows * sm_sizeVertexArrow;
    vbv.StrideInBytes = sm_sizeVertexArrow;
    graphicsCommandListField->IASetVertexBuffers(0, 1, &vbv);
    D3D12_INDEX_BUFFER_VIEW ibv;
    memset(&ibv, 0, sizeof(D3D12_INDEX_BUFFER_VIEW));
    ibv.BufferLocation = m_pIndexBufferArrows->GetGPUVirtualAddress();
    ibv.SizeInBytes =
        s_nbRow * s_nbCol * sm_nbIndexPerArrows * sm_sizeIndexArrow;
    ibv.Format = DXGI_FORMAT_R16_UINT;
    graphicsCommandListField->IASetIndexBuffer(&ibv);

    graphicsCommandListField->DrawIndexedInstanced(
        s_nbRow * s_nbCol * sm_nbIndexPerArrows, 1, 0, 0, 0);

    dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
        .execute_commandList(graphicsCommandListField);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::setup_advectionPass() {}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::setup_arrowGenerationPass()
{
    // ----------------- Root signature and pipeline state
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;
    HRESULT res;

    std::vector<CD3DX12_ROOT_PARAMETER> vRootParameters;
    vRootParameters.resize(3);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vSamplerRanges;
    vSamplerRanges.resize(1);
    vSamplerRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vTextureRanges;
    vTextureRanges.resize(1);
    vTextureRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vOutputRanges;
    vOutputRanges.resize(1);
    vOutputRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

    vRootParameters[0].InitAsDescriptorTable(1, vSamplerRanges.data());
    vRootParameters[1].InitAsDescriptorTable(1, vTextureRanges.data());
    vRootParameters[2].InitAsDescriptorTable(1, vOutputRanges.data());

    {
        CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
        descRootSignature.Init(vRootParameters.size(), vRootParameters.data(),
                               0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

        dx12::ComPtr<ID3DBlob> rootBlob;
        dx12::ComPtr<ID3DBlob> errorBlob;
        res = D3D12SerializeRootSignature(&descRootSignature,
                                          D3D_ROOT_SIGNATURE_VERSION_1,
                                          &rootBlob, &errorBlob);
        if (FAILED(res))
        {
            if (errorBlob != nullptr)
            {
                mLog_info((char*)errorBlob->GetBufferPointer());
            }
        }

        dx12::check_mhr(device->CreateRootSignature(
            0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(),
            IID_PPV_ARGS(&m_rsArrowGeneration)));
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC fieldPipelineDescCompute{};

    dx12::ComPtr<ID3DBlob> cs_field =
        dx12::compile_shader("data/vectorField.hlsl", "cs_main", "cs_6_0");

    fieldPipelineDescCompute.CS.BytecodeLength  = cs_field->GetBufferSize();
    fieldPipelineDescCompute.CS.pShaderBytecode = cs_field->GetBufferPointer();
    fieldPipelineDescCompute.pRootSignature     = m_rsArrowGeneration.Get();

    dx12::check_mhr(device->CreateComputePipelineState(
        &fieldPipelineDescCompute, IID_PPV_ARGS(&m_psoArrowGeneration)));

    // ----------------- Create Vertex and Index resources
    {
        D3D12_HEAP_PROPERTIES props;
        memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
        props.Type                 = D3D12_HEAP_TYPE_DEFAULT;
        props.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        D3D12_RESOURCE_DESC desc;
        memset(&desc, 0, sizeof(D3D12_RESOURCE_DESC));
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width =
            s_nbRow * s_nbCol * sm_nbVertexPerArrows * sm_sizeVertexArrow;
        desc.Height           = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels        = 1;
        desc.Format           = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags            = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        if (device->CreateCommittedResource(
                &props, D3D12_HEAP_FLAG_NONE, &desc,
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr,
                IID_PPV_ARGS(&m_pVertexBufferArrows)) < 0)
            return;

        D3D12_UNORDERED_ACCESS_VIEW_DESC descUAV = {};
        descUAV.Format                           = DXGI_FORMAT_UNKNOWN;
        descUAV.ViewDimension                    = D3D12_UAV_DIMENSION_BUFFER;
        descUAV.Buffer.StructureByteStride       = sm_sizeVertexArrow;
        descUAV.Buffer.NumElements = sm_nbVertexPerArrows * s_nbCol * s_nbRow;

        // CPU Descriptor for the output buffer
        CD3DX12_CPU_DESCRIPTOR_HANDLE const hdlCPUSrv(
            m_pSrvHeap->GetCPUDescriptorHandleForHeapStart(),
            dx12::DX12Surface::scm_numFrames, m_incrementSizeSrv);

        device->CreateUnorderedAccessView(m_pVertexBufferArrows.Get(), nullptr,
                                          &descUAV, hdlCPUSrv);
    }

    {
        D3D12_HEAP_PROPERTIES props;
        memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
        props.Type                 = D3D12_HEAP_TYPE_DEFAULT;
        props.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        D3D12_RESOURCE_DESC desc;
        memset(&desc, 0, sizeof(D3D12_RESOURCE_DESC));
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width =
            s_nbRow * s_nbCol * sm_nbIndexPerArrows * sm_sizeIndexArrow;
        desc.Height           = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels        = 1;
        desc.Format           = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags            = D3D12_RESOURCE_FLAG_NONE;
        if (device->CreateCommittedResource(
                &props, D3D12_HEAP_FLAG_NONE, &desc,
                D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                IID_PPV_ARGS(&m_pIndexBufferArrows)) < 0)
            return;
    }

    // ----------------- Generate arrow indicies
    // Upload generated indices
    ID3D12Resource* pUploadIndexResource;
    D3D12_HEAP_PROPERTIES        props;
    memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
    props.Type                 = D3D12_HEAP_TYPE_UPLOAD;
    props.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    D3D12_RESOURCE_DESC desc;
    memset(&desc, 0, sizeof(D3D12_RESOURCE_DESC));
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width  = s_nbRow * s_nbCol * sm_nbIndexPerArrows * sm_sizeIndexArrow;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels        = 1;
    desc.Format           = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags            = D3D12_RESOURCE_FLAG_NONE;
    if (device->CreateCommittedResource(
            &props, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&pUploadIndexResource)) < 0)
        return;

    // Upload vertex/index data into a single contiguous GPU buffer
    void*       idxResource;
    D3D12_RANGE range;
    memset(&range, 0, sizeof(D3D12_RANGE));
    if (pUploadIndexResource->Map(0, &range, &idxResource) != S_OK)
    {
        mLog_error("Could not map index buffer");
        return;
    }

    auto idxDest = (mU16*)idxResource;

    mU16 indices[s_nbRow * s_nbCol * sm_nbIndexPerArrows];
    for (mUInt i = 0, vIdx = 0; i < s_nbRow * s_nbCol * sm_nbIndexPerArrows;
         i += sm_nbIndexPerArrows, vIdx += sm_nbVertexPerArrows)
    {
        indices[i]     = vIdx;
        indices[i + 1] = vIdx + 1;
        indices[i + 2] = vIdx + 2;
        indices[i + 3] = 0xFFFF;
        indices[i + 4] = vIdx + 1;
        indices[i + 5] = vIdx + 3;
        indices[i + 6] = 0xFFFF;
    }

    memcpy(idxDest, indices,
           s_nbRow * s_nbCol * sm_nbIndexPerArrows * sm_sizeIndexArrow);

    pUploadIndexResource->Unmap(0, &range);

    dx12::ComPtr<ID3D12GraphicsCommandList2> pUploadCommandList =
        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .get_commandList();

    pUploadCommandList->CopyResource(m_pIndexBufferArrows.Get(),
                                     pUploadIndexResource);

    auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_pIndexBufferArrows.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_INDEX_BUFFER);
    pUploadCommandList->ResourceBarrier(1, &resourceBarrier);

    dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
        .execute_commandList(pUploadCommandList.Get());

    // TODO : Why doesn't this work on with clion ??
    //pUploadIndexResource->Release();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::setup_fluidRenderingPass()
{
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;
    HRESULT res;

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

    {
        CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
        descRootSignature.Init(vRootParameters.size(), vRootParameters.data(),
                               0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

        dx12::ComPtr<ID3DBlob> rootBlob;
        dx12::ComPtr<ID3DBlob> errorBlob;
        res = D3D12SerializeRootSignature(&descRootSignature,
                                          D3D_ROOT_SIGNATURE_VERSION_1,
                                          &rootBlob, &errorBlob);
        if (FAILED(res))
        {
            if (errorBlob != nullptr)
            {
                mLog_info((char*)errorBlob->GetBufferPointer());
            }
        }

        dx12::check_mhr(device->CreateRootSignature(
            0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(),
            IID_PPV_ARGS(&m_rsFluidRendering)));
    }

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

    pipelineDesc.pRootSignature = m_rsFluidRendering.Get();

    dx12::check_mhr(device->CreateGraphicsPipelineState(
        &pipelineDesc, IID_PPV_ARGS(&m_psoFluidRendering)));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::setup_arrowRenderingPass()
{
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;
    HRESULT res;

    {
        CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
        descRootSignature.Init(
            0, nullptr, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        dx12::ComPtr<ID3DBlob> rootBlob;
        dx12::ComPtr<ID3DBlob> errorBlob;
        res = D3D12SerializeRootSignature(&descRootSignature,
                                          D3D_ROOT_SIGNATURE_VERSION_1,
                                          &rootBlob, &errorBlob);
        if (FAILED(res))
        {
            if (errorBlob != nullptr)
            {
                mLog_info((char*)errorBlob->GetBufferPointer());
            }
        }

        dx12::check_mhr(device->CreateRootSignature(
            0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(),
            IID_PPV_ARGS(&m_rsArrowRendering)));
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC fieldPipelineDesc{};

    dx12::ComPtr<ID3DBlob> vs_field =
        dx12::compile_shader("data/vectorField.hlsl", "vs_main", "vs_6_0");
    dx12::ComPtr<ID3DBlob> ps_field =
        dx12::compile_shader("data/vectorField.hlsl", "ps_main", "ps_6_0");

    const D3D12_INPUT_ELEMENT_DESC inputElements[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
         D3D12_APPEND_ALIGNED_ELEMENT,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
         D3D12_APPEND_ALIGNED_ELEMENT,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

    fieldPipelineDesc.InputLayout.pInputElementDescs = inputElements;
    fieldPipelineDesc.InputLayout.NumElements        = std::size(inputElements);
    fieldPipelineDesc.IBStripCutValue =
        D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;

    fieldPipelineDesc.VS.BytecodeLength  = vs_field->GetBufferSize();
    fieldPipelineDesc.VS.pShaderBytecode = vs_field->GetBufferPointer();
    fieldPipelineDesc.PS.BytecodeLength  = ps_field->GetBufferSize();
    fieldPipelineDesc.PS.pShaderBytecode = ps_field->GetBufferPointer();

    fieldPipelineDesc.NumRenderTargets = 1;
    fieldPipelineDesc.RTVFormats[0]    = DXGI_FORMAT_B8G8R8A8_UNORM;
    fieldPipelineDesc.BlendState       = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    fieldPipelineDesc.BlendState.RenderTarget[0].BlendEnable = 1;
    fieldPipelineDesc.BlendState.RenderTarget[0].SrcBlend =
        D3D12_BLEND_SRC_ALPHA;
    fieldPipelineDesc.BlendState.RenderTarget[0].DestBlend =
        D3D12_BLEND_INV_SRC_ALPHA;
    fieldPipelineDesc.BlendState.RenderTarget[0].SrcBlendAlpha =
        D3D12_BLEND_SRC_ALPHA;
    fieldPipelineDesc.BlendState.RenderTarget[0].DestBlendAlpha =
        D3D12_BLEND_INV_SRC_ALPHA;
    fieldPipelineDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    fieldPipelineDesc.BlendState.RenderTarget[0].BlendOpAlpha =
        D3D12_BLEND_OP_ADD;

    fieldPipelineDesc.SampleMask = 0xFFFFFFFF;

    fieldPipelineDesc.RasterizerState.SlopeScaledDepthBias  = 0;
    fieldPipelineDesc.RasterizerState.DepthClipEnable       = false;
    fieldPipelineDesc.RasterizerState.MultisampleEnable     = false;
    fieldPipelineDesc.RasterizerState.AntialiasedLineEnable = false;
    fieldPipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    fieldPipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    fieldPipelineDesc.RasterizerState.FrontCounterClockwise = true;
    fieldPipelineDesc.RasterizerState.DepthBias             = 0;
    fieldPipelineDesc.RasterizerState.DepthBiasClamp        = 0.0;

    fieldPipelineDesc.PrimitiveTopologyType =
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    fieldPipelineDesc.SampleDesc.Count = 1;

    fieldPipelineDesc.pRootSignature = m_rsArrowRendering.Get();
    dx12::check_mhr(device->CreateGraphicsPipelineState(
        &fieldPipelineDesc, IID_PPV_ARGS(&m_psoArrowRendering)));
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