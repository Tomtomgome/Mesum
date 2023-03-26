#include "RenderTaskFluidSimulation.hpp"

using namespace m;
using namespace m::render;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void DescriptorHeapFluidSimulation::init()
{
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;

    m_incrementSizeSrv = device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_DESCRIPTOR_HEAP_DESC sSrvHeapDesc = {};
    sSrvHeapDesc.NumDescriptors             = scm_maxSizeDescriptorHeap;
    sSrvHeapDesc.Type     = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    sSrvHeapDesc.NodeMask = 0;
    sSrvHeapDesc.Flags    = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    HRESULT hr = device->CreateDescriptorHeap(
        &sSrvHeapDesc, IID_PPV_ARGS(m_pHeap.GetAddressOf()));
    mAssert(hr == S_OK);
    m_pHeap.Get()->SetName(L"SRV Heap");
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE
DescriptorHeapFluidSimulation::create_srvAndGetHandle(
    m::dx12::ComPtr<ID3D12Resource>& a_pResource,
    D3D12_SHADER_RESOURCE_VIEW_DESC& a_descSrv)
{
    mAssert(m_currentHandle !=
            scm_maxSizeDescriptorHeap);  // Max heap size reached
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;

    D3D12_GPU_DESCRIPTOR_HANDLE hdlGPU = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        m_pHeap->GetGPUDescriptorHandleForHeapStart(), m_currentHandle,
        m_incrementSizeSrv);

    CD3DX12_CPU_DESCRIPTOR_HANDLE const hdlCPU(
        m_pHeap->GetCPUDescriptorHandleForHeapStart(), m_currentHandle,
        m_incrementSizeSrv);
    device->CreateShaderResourceView(a_pResource.Get(), &a_descSrv, hdlCPU);

    m_currentHandle++;

    return hdlGPU;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
D3D12_GPU_DESCRIPTOR_HANDLE
DescriptorHeapFluidSimulation::create_uavAndGetHandle(
    m::dx12::ComPtr<ID3D12Resource>&  a_pResource,
    D3D12_UNORDERED_ACCESS_VIEW_DESC& a_descUav)
{
    mAssert(m_currentHandle !=
            scm_maxSizeDescriptorHeap);  // Max heap size reached
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;

    D3D12_GPU_DESCRIPTOR_HANDLE hdlGPU = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        m_pHeap->GetGPUDescriptorHandleForHeapStart(), m_currentHandle,
        m_incrementSizeSrv);

    CD3DX12_CPU_DESCRIPTOR_HANDLE const hdlCPU(
        m_pHeap->GetCPUDescriptorHandleForHeapStart(), m_currentHandle,
        m_incrementSizeSrv);

    device->CreateUnorderedAccessView(a_pResource.Get(), nullptr, &a_descUav,
                                      hdlCPU);

    m_currentHandle++;

    return hdlGPU;
}

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
    descTexture.Flags              = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
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
    const UINT64 uploadBufferSize = GetRequiredIntermediateSize(
        a_pTextureResource.Get(), 0, subresourceCount);

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

    resource::mTypedImage<math::mVec4> const& rImage =
        unref_safe(m_taskData.pInitialData);

    m_simulationWidth  = rImage.width;
    m_simulationHeight = rImage.height;

    m_descriptorHeap.init();

    init_samplers();

    init_velocityTextures();

    init_dataTextures(rImage);

    init_constantBuffer();

    // -------------------- Root Signatures
    setup_velocityAdvectionPass();

    setup_simulationPass();

    setup_advectionPass();

    setup_jacobiPass();

    setup_projectionPass();

    setup_fluidRenderingPass();

    setup_arrowGenerationPass();

    setup_arrowRenderingPass();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::prepare() {}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::execute() const
{
    ID3D12Resource* pTextureResourceOriginal =
        m_pTextureResources[m_iOriginal].Get();
    ID3D12Resource* pTextureResource = m_pTextureResources[m_iComputed].Get();

    auto pOutputRT =
        static_cast<dx12::mRenderTarget const*>(m_taskData.pOutputRT);

    auto parameters =
        static_cast<TaskDataFluidSimulation::ControlParameters const&>(
            unref_safe(m_taskData.pParameters));
    mInt screenWidth  = 640;
    mInt screenHeight = 640;

    D3D12_CPU_DESCRIPTOR_HANDLE rtv;
    rtv                     = pOutputRT->rtv;
    D3D12_VIEWPORT viewport = {};
    viewport.MaxDepth       = 1.0f;
    viewport.Width          = screenWidth;
    viewport.Height         = screenHeight;
    D3D12_RECT scissorRect  = {};
    scissorRect.right       = screenWidth;
    scissorRect.bottom      = screenHeight;

    ID3D12DescriptorHeap* const aHeaps[1] = {m_descriptorHeap.m_pHeap.Get()};

    D3D12_RESOURCE_BARRIER resourceBarrier[2];

    mUInt nbComputeRow = m_simulationHeight;
    mUInt nbComputeCol = m_simulationWidth;

    mU8*  m_pBufferData = (mU8*)(m_pConstantBuffersData);
    mUInt offset        = 0;

    mUInt          offsetResolutionArrows = offset;
    math::mUIVec2& resolutionArrows       = *((math::mUIVec2*)(m_pBufferData));
    resolutionArrows.x                    = std::max(
                           mUInt(16), std::min(mUInt(parameters.vectorRepresentationResolution.x),
                                               scm_arrowFieldMaxResolutionX));
    resolutionArrows.y = std::max(
        mUInt(16), std::min(mUInt(parameters.vectorRepresentationResolution.y),
                            scm_arrowFieldMaxResolutionY));
    offset += round_up(sizeof(math::mUIVec2), scm_minimalStructSize) *
              scm_minimalStructSize;
    m_pBufferData += offset;

    mUInt          offsetResolutionBaseImage = offset;
    math::mUIVec2& resolutionBaseImage = *((math::mUIVec2*)(m_pBufferData));
    resolutionBaseImage = math::mUIVec2{m_simulationWidth, m_simulationHeight};

    if (parameters.isRunning)
    {
        // ---------------- Velocity advection
        {
            dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
                dx12::DX12Context::gs_dx12Contexte->get_computeCommandQueue()
                    .get_commandList();

            computeCommandList->SetName(L"Velocity advection command list");

            computeCommandList->SetDescriptorHeaps(1, aHeaps);
            computeCommandList->SetComputeRootSignature(
                m_rsVelocityAdvection.Get());

            computeCommandList->SetPipelineState(m_psoVelocityAdvection.Get());

            computeCommandList->SetComputeRootDescriptorTable(
                0, m_GPUDescHdlVelocityInput[m_iOriginal]);
            computeCommandList->SetComputeRootDescriptorTable(
                1, m_GPUDescHdlVelocityOutput[m_iComputed]);

            computeCommandList->SetComputeRootConstantBufferView(
                2, m_pConstantBuffers->GetGPUVirtualAddress() +
                       offsetResolutionBaseImage);

            computeCommandList->Dispatch(
                round_up<mUInt>(nbComputeCol, m_sizeComputeGroup),
                round_up<mUInt>(nbComputeRow, m_sizeComputeGroup), 1);

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceVelocity[m_iOriginal].Get(),
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            resourceBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceVelocity[m_iComputed].Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            computeCommandList->ResourceBarrier(2, resourceBarrier);

            computeCommandList->SetPipelineState(m_psoVelocityStaggering.Get());

            computeCommandList->SetComputeRootDescriptorTable(
                0, m_GPUDescHdlVelocityInput[m_iComputed]);
            computeCommandList->SetComputeRootDescriptorTable(
                1, m_GPUDescHdlVelocityOutput[m_iOriginal]);

            computeCommandList->Dispatch(
                round_up<mUInt>(nbComputeCol, m_sizeComputeGroup),
                round_up<mUInt>(nbComputeRow, m_sizeComputeGroup), 1);

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceVelocity[m_iComputed].Get(),
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            resourceBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceVelocity[m_iOriginal].Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            computeCommandList->ResourceBarrier(2, resourceBarrier);

            dx12::DX12Context::gs_dx12Contexte->get_computeCommandQueue()
                .execute_commandList(computeCommandList);
        }

        //*
        // ---------------- Simulate forces
        {
            dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
                dx12::DX12Context::gs_dx12Contexte->get_computeCommandQueue()
                    .get_commandList();

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                pTextureResourceOriginal,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            computeCommandList->ResourceBarrier(1, resourceBarrier);

            computeCommandList->SetDescriptorHeaps(1, aHeaps);

            computeCommandList->SetPipelineState(m_psoSimulation.Get());
            computeCommandList->SetComputeRootSignature(m_rsSimulation.Get());

            computeCommandList->SetComputeRootDescriptorTable(
                0, m_GPUDescHdlTextureDisplay[m_iOriginal]);
            computeCommandList->SetComputeRootDescriptorTable(
                1, m_GPUDescHdlVelocityInput[m_iOriginal]);
            computeCommandList->SetComputeRootDescriptorTable(
                2, m_GPUDescHdlVelocityOutput[m_iComputed]);

            computeCommandList->Dispatch(nbComputeCol, nbComputeRow, 1);

            dx12::DX12Context::gs_dx12Contexte->get_computeCommandQueue()
                .execute_commandList(computeCommandList);
        }
        //*
        // ---------------- Projection
        // ------- Jacobi
        {
            dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
                dx12::DX12Context::gs_dx12Contexte->get_computeCommandQueue()
                    .get_commandList();

            computeCommandList->SetDescriptorHeaps(1, aHeaps);

            computeCommandList->SetPipelineState(m_psoJacobi.Get());
            computeCommandList->SetComputeRootSignature(m_rsJacobi.Get());

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceVelocity[m_iComputed].Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            computeCommandList->ResourceBarrier(1, resourceBarrier);

            computeCommandList->SetComputeRootDescriptorTable(
                0, m_GPUDescHdlVelocityInput[m_iComputed]);

            for (int i = 0; i < scm_nbJacobiIteration; ++i)
            {
                computeCommandList->SetComputeRootDescriptorTable(
                    1, m_GPUDescHdlJacobiInput[i % 2]);
                computeCommandList->SetComputeRootDescriptorTable(
                    2, m_GPUDescHdlJacobiOutput[(i + 1) % 2]);

                computeCommandList->Dispatch(nbComputeRow, nbComputeCol, 1);

                resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                    m_pTextureResourceJacobi[i % 2].Get(),
                    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                resourceBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(
                    m_pTextureResourceJacobi[(i + 1) % 2].Get(),
                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                computeCommandList->ResourceBarrier(2, resourceBarrier);
            }

            dx12::DX12Context::gs_dx12Contexte->get_computeCommandQueue()
                .execute_commandList(computeCommandList);
        }
        //*
        // ------- Pressure application
        {
            dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
                dx12::DX12Context::gs_dx12Contexte->get_computeCommandQueue()
                    .get_commandList();

            computeCommandList->SetDescriptorHeaps(1, aHeaps);

            computeCommandList->SetPipelineState(m_psoProject.Get());
            computeCommandList->SetComputeRootSignature(m_rsProject.Get());

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceVelocity[m_iComputed].Get(),
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            computeCommandList->ResourceBarrier(1, resourceBarrier);

            computeCommandList->SetComputeRootDescriptorTable(
                0, m_GPUDescHdlJacobiInput[0]);
            computeCommandList->SetComputeRootDescriptorTable(
                1, m_GPUDescHdlVelocityOutput[m_iComputed]);

            computeCommandList->Dispatch(nbComputeCol, nbComputeRow, 1);

            dx12::DX12Context::gs_dx12Contexte->get_computeCommandQueue()
                .execute_commandList(computeCommandList);
        }
        //*/
        // ---------------- Generate arrows part
        {
            dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
                dx12::DX12Context::gs_dx12Contexte->get_computeCommandQueue()
                    .get_commandList();

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceVelocity[m_iComputed].Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            resourceBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pVertexBufferArrows.Get(),
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            computeCommandList->ResourceBarrier(2, resourceBarrier);

            computeCommandList->SetDescriptorHeaps(1, aHeaps);

            computeCommandList->SetPipelineState(m_psoArrowGeneration.Get());
            computeCommandList->SetComputeRootSignature(
                m_rsArrowGeneration.Get());

            computeCommandList->SetComputeRootDescriptorTable(
                0, m_GPUDescHdlVelocityInput[m_iComputed]);
            computeCommandList->SetComputeRootDescriptorTable(
                1, m_GPUDescHdlOutBuffer);

            computeCommandList->SetComputeRootConstantBufferView(
                2, m_pConstantBuffers->GetGPUVirtualAddress() +
                       offsetResolutionArrows);

            computeCommandList->Dispatch(
                round_up<mUInt>(parameters.vectorRepresentationResolution.x,
                                m_sizeComputeGroup),
                round_up<mUInt>(parameters.vectorRepresentationResolution.y,
                                m_sizeComputeGroup),
                1);

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pVertexBufferArrows.Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
            computeCommandList->ResourceBarrier(1, resourceBarrier);

            dx12::DX12Context::gs_dx12Contexte->get_computeCommandQueue()
                .execute_commandList(computeCommandList);
        }
        // ---------------- Advection
        {
            dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
                dx12::DX12Context::gs_dx12Contexte->get_computeCommandQueue()
                    .get_commandList();

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                pTextureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            computeCommandList->ResourceBarrier(1, resourceBarrier);

            computeCommandList->SetDescriptorHeaps(1, aHeaps);

            computeCommandList->SetPipelineState(m_psoAdvection.Get());
            computeCommandList->SetComputeRootSignature(m_rsAdvection.Get());

            computeCommandList->SetComputeRootDescriptorTable(
                0, m_GPUDescHdlTextureDisplay[m_iOriginal]);
            computeCommandList->SetComputeRootDescriptorTable(
                1, m_GPUDescHdlVelocityInput[m_iComputed]);
            computeCommandList->SetComputeRootDescriptorTable(
                2, m_GPUDescHdlTextureCompute[m_iComputed]);

            computeCommandList->Dispatch(nbComputeCol, nbComputeRow, 1);

            dx12::DX12Context::gs_dx12Contexte->get_computeCommandQueue()
                .execute_commandList(computeCommandList);
        }

        // Copies to original textures
        {
            dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
                dx12::DX12Context::gs_dx12Contexte->get_computeCommandQueue()
                    .get_commandList();

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                pTextureResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_COPY_SOURCE);
            resourceBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(
                pTextureResourceOriginal,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_COPY_DEST);
            computeCommandList->ResourceBarrier(2, resourceBarrier);

            computeCommandList->CopyResource(pTextureResourceOriginal,
                                             pTextureResource);

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                pTextureResource, D3D12_RESOURCE_STATE_COPY_SOURCE,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            resourceBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(
                pTextureResourceOriginal, D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            computeCommandList->ResourceBarrier(2, resourceBarrier);

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceVelocity[m_iComputed].Get(),
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_COPY_SOURCE);
            resourceBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceVelocity[m_iOriginal].Get(),
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_COPY_DEST);
            computeCommandList->ResourceBarrier(2, resourceBarrier);

            computeCommandList->CopyResource(
                m_pTextureResourceVelocity[m_iOriginal].Get(),
                m_pTextureResourceVelocity[m_iComputed].Get());

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceVelocity[m_iComputed].Get(),
                D3D12_RESOURCE_STATE_COPY_SOURCE,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            resourceBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceVelocity[m_iOriginal].Get(),
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            computeCommandList->ResourceBarrier(2, resourceBarrier);

            dx12::DX12Context::gs_dx12Contexte->get_computeCommandQueue()
                .execute_commandList(computeCommandList);
        }
    }
    // ---------------- Renders the fluid
    {
        dx12::ComPtr<ID3D12GraphicsCommandList2> graphicCommandList =
            dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                .get_commandList();

        graphicCommandList->SetDescriptorHeaps(1, aHeaps);

        graphicCommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
        graphicCommandList->RSSetViewports(1, &viewport);
        graphicCommandList->RSSetScissorRects(1, &scissorRect);

        graphicCommandList->SetPipelineState(m_psoFluidRendering.Get());
        graphicCommandList->SetGraphicsRootSignature(m_rsFluidRendering.Get());

        dx12::ComPtr<ID3D12Device> device =
            dx12::DX12Context::gs_dx12Contexte->m_device;

        graphicCommandList->IASetPrimitiveTopology(
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        graphicCommandList->SetGraphicsRootDescriptorTable(
            0, m_GPUDescHdlTextureDisplay[m_iOriginal]);

        graphicCommandList->DrawInstanced(3, 1, 0, 0);

        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .execute_commandList(graphicCommandList.Get());
    }

    // ---------------- Render Arrow
    if (parameters.displaySpeed)
    {
        dx12::ComPtr<ID3D12GraphicsCommandList2> graphicsCommandListField =
            dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                .get_commandList();

        graphicsCommandListField->SetDescriptorHeaps(1, aHeaps);

        graphicsCommandListField->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
        graphicsCommandListField->RSSetViewports(1, &viewport);
        graphicsCommandListField->RSSetScissorRects(1, &scissorRect);

        graphicsCommandListField->SetPipelineState(m_psoArrowRendering.Get());
        graphicsCommandListField->SetGraphicsRootSignature(
            m_rsArrowRendering.Get());

        graphicsCommandListField->IASetPrimitiveTopology(
            D3D_PRIMITIVE_TOPOLOGY_LINESTRIP);

        D3D12_VERTEX_BUFFER_VIEW vbv;
        memset(&vbv, 0, sizeof(D3D12_VERTEX_BUFFER_VIEW));
        vbv.BufferLocation = m_pVertexBufferArrows->GetGPUVirtualAddress();
        vbv.SizeInBytes    = parameters.vectorRepresentationResolution.x *
                          parameters.vectorRepresentationResolution.y *
                          sm_nbVertexPerArrows * sm_sizeVertexArrow;
        vbv.StrideInBytes = sm_sizeVertexArrow;
        graphicsCommandListField->IASetVertexBuffers(0, 1, &vbv);
        D3D12_INDEX_BUFFER_VIEW ibv;
        memset(&ibv, 0, sizeof(D3D12_INDEX_BUFFER_VIEW));
        ibv.BufferLocation = m_pIndexBufferArrows->GetGPUVirtualAddress();
        ibv.SizeInBytes    = parameters.vectorRepresentationResolution.x *
                          parameters.vectorRepresentationResolution.y *
                          sm_nbIndexPerArrows * sm_sizeIndexArrow;
        ibv.Format = DXGI_FORMAT_R32_UINT;
        graphicsCommandListField->IASetIndexBuffer(&ibv);

        graphicsCommandListField->DrawIndexedInstanced(
            parameters.vectorRepresentationResolution.x *
                parameters.vectorRepresentationResolution.y *
                sm_nbIndexPerArrows,
            1, 0, 0, 0);

        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .execute_commandList(graphicsCommandListField);
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::init_samplers()
{
    mUInt registerID = 0;

    D3D12_STATIC_SAMPLER_DESC descStaticSampler = {};
    descStaticSampler.Filter         = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    descStaticSampler.MinLOD         = 0;
    descStaticSampler.MaxLOD         = 0;
    descStaticSampler.MipLODBias     = 0.0f;
    descStaticSampler.MaxAnisotropy  = 1;
    descStaticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

    {
        D3D12_FILTER_TYPE eDx12FilterMinMag = D3D12_FILTER_TYPE_LINEAR;
        D3D12_FILTER_TYPE eDx12FilterMip    = D3D12_FILTER_TYPE_LINEAR;
        D3D12_FILTER_REDUCTION_TYPE eDx12FilterReduction =
            D3D12_FILTER_REDUCTION_TYPE_STANDARD;
        descStaticSampler.Filter =
            D3D12_ENCODE_BASIC_FILTER(eDx12FilterMinMag, eDx12FilterMinMag,
                                      eDx12FilterMip, eDx12FilterReduction);

        D3D12_TEXTURE_ADDRESS_MODE eDx12AddressMode =
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        descStaticSampler.AddressU = eDx12AddressMode;
        descStaticSampler.AddressV = eDx12AddressMode;
        descStaticSampler.AddressW = eDx12AddressMode;
        descStaticSampler.BorderColor =
            D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;

        descStaticSampler.ShaderRegister = registerID++;
        descStaticSampler.RegisterSpace  = 0;
        m_samplersDescs.push_back(descStaticSampler);
    }

    {
        D3D12_FILTER_TYPE           eDx12FilterMinMag = D3D12_FILTER_TYPE_POINT;
        D3D12_FILTER_TYPE           eDx12FilterMip    = D3D12_FILTER_TYPE_POINT;
        D3D12_FILTER_REDUCTION_TYPE eDx12FilterReduction =
            D3D12_FILTER_REDUCTION_TYPE_STANDARD;
        descStaticSampler.Filter =
            D3D12_ENCODE_BASIC_FILTER(eDx12FilterMinMag, eDx12FilterMinMag,
                                      eDx12FilterMip, eDx12FilterReduction);

        D3D12_TEXTURE_ADDRESS_MODE eDx12AddressMode =
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        descStaticSampler.AddressU = eDx12AddressMode;
        descStaticSampler.AddressV = eDx12AddressMode;
        descStaticSampler.AddressW = eDx12AddressMode;
        descStaticSampler.BorderColor =
            D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;

        descStaticSampler.ShaderRegister = registerID++;
        descStaticSampler.RegisterSpace  = 0;
        m_samplersDescs.push_back(descStaticSampler);
    }

    {
        D3D12_FILTER_TYPE           eDx12FilterMinMag = D3D12_FILTER_TYPE_POINT;
        D3D12_FILTER_TYPE           eDx12FilterMip    = D3D12_FILTER_TYPE_POINT;
        D3D12_FILTER_REDUCTION_TYPE eDx12FilterReduction =
            D3D12_FILTER_REDUCTION_TYPE_STANDARD;
        descStaticSampler.Filter =
            D3D12_ENCODE_BASIC_FILTER(eDx12FilterMinMag, eDx12FilterMinMag,
                                      eDx12FilterMip, eDx12FilterReduction);

        D3D12_TEXTURE_ADDRESS_MODE eDx12AddressMode =
            D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        descStaticSampler.AddressU = eDx12AddressMode;
        descStaticSampler.AddressV = eDx12AddressMode;
        descStaticSampler.AddressW = eDx12AddressMode;
        descStaticSampler.BorderColor =
            D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;

        descStaticSampler.ShaderRegister = registerID++;
        descStaticSampler.RegisterSpace  = 0;
        m_samplersDescs.push_back(descStaticSampler);
    }

    {
        D3D12_FILTER_TYPE eDx12FilterMinMag = D3D12_FILTER_TYPE_LINEAR;
        D3D12_FILTER_TYPE eDx12FilterMip    = D3D12_FILTER_TYPE_LINEAR;
        D3D12_FILTER_REDUCTION_TYPE eDx12FilterReduction =
            D3D12_FILTER_REDUCTION_TYPE_STANDARD;
        descStaticSampler.Filter =
            D3D12_ENCODE_BASIC_FILTER(eDx12FilterMinMag, eDx12FilterMinMag,
                                      eDx12FilterMip, eDx12FilterReduction);

        D3D12_TEXTURE_ADDRESS_MODE eDx12AddressMode =
            D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        descStaticSampler.AddressU = eDx12AddressMode;
        descStaticSampler.AddressV = eDx12AddressMode;
        descStaticSampler.AddressW = eDx12AddressMode;
        descStaticSampler.BorderColor =
            D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;

        descStaticSampler.ShaderRegister = registerID++;
        descStaticSampler.RegisterSpace  = 0;
        m_samplersDescs.push_back(descStaticSampler);
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::init_velocityTextures()
{
    m_pTextureResourceVelocity.resize(scm_nbVelocityTexture);

    D3D12_RESOURCE_DESC descTexture{};
    descTexture.MipLevels          = 1;
    descTexture.Width              = m_simulationWidth;
    descTexture.Height             = m_simulationHeight;
    descTexture.Flags              = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    descTexture.DepthOrArraySize   = 1;
    descTexture.SampleDesc.Count   = 1;
    descTexture.SampleDesc.Quality = 0;
    descTexture.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    descTexture.Format             = scm_formatVelocityTexture;

    D3D12_SHADER_RESOURCE_VIEW_DESC descSrv = {};
    descSrv.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
    descSrv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    descSrv.Format                  = scm_formatVelocityTexture;
    descSrv.Texture2D.MipLevels     = 1;
    descSrv.Texture2D.MostDetailedMip     = 0;
    descSrv.Texture2D.ResourceMinLODClamp = 0.0f;

    D3D12_UNORDERED_ACCESS_VIEW_DESC descUav = {};
    descUav.Format                           = scm_formatVelocityTexture;
    descUav.ViewDimension                    = D3D12_UAV_DIMENSION_TEXTURE2D;

    ID3D12Device2* pDevice = dx12::DX12Context::gs_dx12Contexte->m_device.Get();

    auto oHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    HRESULT hr = pDevice->CreateCommittedResource(
        &oHeapProperties, D3D12_HEAP_FLAG_NONE, &descTexture,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, nullptr,
        IID_PPV_ARGS(m_pTextureResourceVelocity[0].GetAddressOf()));
    mAssert(hr == S_OK);
    m_pTextureResourceVelocity[0]->SetName(L"Velocity Texture 0");
    m_GPUDescHdlVelocityInput[0] = m_descriptorHeap.create_srvAndGetHandle(
        m_pTextureResourceVelocity[0], descSrv);
    m_GPUDescHdlVelocityOutput[0] = m_descriptorHeap.create_uavAndGetHandle(
        m_pTextureResourceVelocity[0], descUav);

    hr = pDevice->CreateCommittedResource(
        &oHeapProperties, D3D12_HEAP_FLAG_NONE, &descTexture,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
        IID_PPV_ARGS(m_pTextureResourceVelocity[1].GetAddressOf()));
    mAssert(hr == S_OK);
    m_pTextureResourceVelocity[1]->SetName(L"Velocity Texture 1");
    m_GPUDescHdlVelocityInput[1] = m_descriptorHeap.create_srvAndGetHandle(
        m_pTextureResourceVelocity[1], descSrv);
    m_GPUDescHdlVelocityOutput[1] = m_descriptorHeap.create_uavAndGetHandle(
        m_pTextureResourceVelocity[1], descUav);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::init_dataTextures(
    m::resource::mTypedImage<m::math::mVec4> const& a_rImage)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC descShaderResourceView = {};
    descShaderResourceView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    descShaderResourceView.Shader4ComponentMapping =
        D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    descShaderResourceView.Format                    = scm_formatDataTexture;
    descShaderResourceView.Texture2D.MipLevels       = 1;
    descShaderResourceView.Texture2D.MostDetailedMip = 0;
    descShaderResourceView.Texture2D.ResourceMinLODClamp = 0.0f;

    D3D12_UNORDERED_ACCESS_VIEW_DESC descUav = {};
    descUav.Format                           = scm_formatDataTexture;
    descUav.ViewDimension                    = D3D12_UAV_DIMENSION_TEXTURE2D;
    for (int i = 0; i < scm_nbDataTextures; ++i)
    {
        m_pTextureResources.emplace_back();
        m_pUploadResourcesToDelete.emplace_back();

        auto [result, _] = upload_toGPU(
            a_rImage, m_pTextureResources.back(),
            m_pUploadResourcesToDelete.back(), scm_formatDataTexture,
            std::wstring(L"Texture_" + std::to_wstring(i)).c_str());
        mAssert(mIsSuccess(result));

        m_GPUDescHdlTextureDisplay[i] = m_descriptorHeap.create_srvAndGetHandle(
            m_pTextureResources.back(), descShaderResourceView);
        m_GPUDescHdlTextureCompute[i] = m_descriptorHeap.create_uavAndGetHandle(
            m_pTextureResources.back(), descUav);
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::init_constantBuffer()
{
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;
    // TODO needs to do for each frame
    {
        auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto resourceDesc =
            CD3DX12_RESOURCE_DESC::Buffer(scm_maxSizeConstantBuffer);
        if (device->CreateCommittedResource(
                &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&m_pConstantBuffers)) < 0)
            return;
        m_pConstantBuffers->SetName(L"Constant Buffer");

        CD3DX12_RANGE readRange(0, 0);
        dx12::check_mhr(
            m_pConstantBuffers->Map(0, &readRange, &m_pConstantBuffersData));
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::setup_velocityAdvectionPass()
{
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;
    HRESULT res;
    // ----------------- Root signature and pipeline state
    std::vector<CD3DX12_ROOT_PARAMETER> vRootParameters;
    vRootParameters.resize(3);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vTextureRanges;
    vTextureRanges.resize(1);
    vTextureRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vOutputRanges;
    vOutputRanges.resize(1);
    vOutputRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

    vRootParameters[0].InitAsDescriptorTable(1, vTextureRanges.data());
    vRootParameters[1].InitAsDescriptorTable(1, vOutputRanges.data());
    vRootParameters[2].InitAsConstantBufferView(0);

    {
        CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
        descRootSignature.Init(vRootParameters.size(), vRootParameters.data(),
                               m_samplersDescs.size(), m_samplersDescs.data(),
                               D3D12_ROOT_SIGNATURE_FLAG_NONE);

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
            IID_PPV_ARGS(&m_rsVelocityAdvection)));
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC fieldPipelineDescCompute{};

    dx12::ComPtr<ID3DBlob> shaderProgram = dx12::compile_shader(
        "data/velocityAdvection.hlsl", "cs_advectVelocityCellCenter", "cs_6_0");

    fieldPipelineDescCompute.CS.BytecodeLength = shaderProgram->GetBufferSize();
    fieldPipelineDescCompute.CS.pShaderBytecode =
        shaderProgram->GetBufferPointer();
    fieldPipelineDescCompute.pRootSignature = m_rsVelocityAdvection.Get();

    dx12::check_mhr(device->CreateComputePipelineState(
        &fieldPipelineDescCompute, IID_PPV_ARGS(&m_psoVelocityAdvection)));

    shaderProgram = dx12::compile_shader("data/velocityAdvection.hlsl",
                                         "cs_staggerVelocities", "cs_6_0");

    fieldPipelineDescCompute.CS.BytecodeLength = shaderProgram->GetBufferSize();
    fieldPipelineDescCompute.CS.pShaderBytecode =
        shaderProgram->GetBufferPointer();
    fieldPipelineDescCompute.pRootSignature = m_rsVelocityAdvection.Get();

    dx12::check_mhr(device->CreateComputePipelineState(
        &fieldPipelineDescCompute, IID_PPV_ARGS(&m_psoVelocityStaggering)));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::setup_advectionPass()
{
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;
    HRESULT res;
    // ----------------- Root signature and pipeline state
    std::vector<CD3DX12_ROOT_PARAMETER> vRootParameters;
    vRootParameters.resize(3);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vTextureRanges;
    vTextureRanges.resize(1);
    vTextureRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vTextureRangesVelocity;
    vTextureRangesVelocity.resize(1);
    vTextureRangesVelocity[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vOutputRanges;
    vOutputRanges.resize(1);
    vOutputRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

    vRootParameters[0].InitAsDescriptorTable(1, vTextureRanges.data());
    vRootParameters[1].InitAsDescriptorTable(1, vTextureRangesVelocity.data());
    vRootParameters[2].InitAsDescriptorTable(1, vOutputRanges.data());

    {
        CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
        descRootSignature.Init(vRootParameters.size(), vRootParameters.data(),
                               m_samplersDescs.size(), m_samplersDescs.data(),
                               D3D12_ROOT_SIGNATURE_FLAG_NONE);

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
            IID_PPV_ARGS(&m_rsAdvection)));
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC fieldPipelineDescCompute{};

    dx12::ComPtr<ID3DBlob> cs_field =
        dx12::compile_shader("data/dataAdvection.hlsl", "cs_advect", "cs_6_0");

    fieldPipelineDescCompute.CS.BytecodeLength  = cs_field->GetBufferSize();
    fieldPipelineDescCompute.CS.pShaderBytecode = cs_field->GetBufferPointer();
    fieldPipelineDescCompute.pRootSignature     = m_rsAdvection.Get();

    dx12::check_mhr(device->CreateComputePipelineState(
        &fieldPipelineDescCompute, IID_PPV_ARGS(&m_psoAdvection)));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::setup_simulationPass()
{
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;
    HRESULT res;
    // ----------------- Root signature and pipeline state
    std::vector<CD3DX12_ROOT_PARAMETER> vRootParameters;
    vRootParameters.resize(3);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vTextureRanges;
    vTextureRanges.resize(1);
    vTextureRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vTextureRangesVelocity;
    vTextureRangesVelocity.resize(1);
    vTextureRangesVelocity[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vOutputRanges;
    vOutputRanges.resize(1);
    vOutputRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

    vRootParameters[0].InitAsDescriptorTable(1, vTextureRanges.data());
    vRootParameters[1].InitAsDescriptorTable(1, vTextureRangesVelocity.data());
    vRootParameters[2].InitAsDescriptorTable(1, vOutputRanges.data());

    {
        CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
        descRootSignature.Init(vRootParameters.size(), vRootParameters.data(),
                               m_samplersDescs.size(), m_samplersDescs.data(),
                               D3D12_ROOT_SIGNATURE_FLAG_NONE);

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
            IID_PPV_ARGS(&m_rsSimulation)));
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC fieldPipelineDescCompute{};

    dx12::ComPtr<ID3DBlob> cs_field = dx12::compile_shader(
        "data/fluidSimulation.hlsl", "cs_simulation", "cs_6_0");

    fieldPipelineDescCompute.CS.BytecodeLength  = cs_field->GetBufferSize();
    fieldPipelineDescCompute.CS.pShaderBytecode = cs_field->GetBufferPointer();
    fieldPipelineDescCompute.pRootSignature     = m_rsSimulation.Get();

    dx12::check_mhr(device->CreateComputePipelineState(
        &fieldPipelineDescCompute, IID_PPV_ARGS(&m_psoSimulation)));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::setup_jacobiPass()
{
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;
    HRESULT res;
    // ----------------- Root signature and pipeline state
    {
        std::vector<CD3DX12_ROOT_PARAMETER> vRootParameters;
        vRootParameters.resize(3);

        std::vector<CD3DX12_DESCRIPTOR_RANGE> vDataTextureRanges;
        vDataTextureRanges.resize(1);
        vDataTextureRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

        std::vector<CD3DX12_DESCRIPTOR_RANGE> vPressureTextureRanges;
        vPressureTextureRanges.resize(1);
        vPressureTextureRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1,
                                       0);

        std::vector<CD3DX12_DESCRIPTOR_RANGE> vOutputRanges;
        vOutputRanges.resize(1);
        vOutputRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

        vRootParameters[0].InitAsDescriptorTable(1, vDataTextureRanges.data());
        vRootParameters[1].InitAsDescriptorTable(1,
                                                 vPressureTextureRanges.data());
        vRootParameters[2].InitAsDescriptorTable(1, vOutputRanges.data());

        {
            CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
            descRootSignature.Init(
                vRootParameters.size(), vRootParameters.data(),
                m_samplersDescs.size(), m_samplersDescs.data(),
                D3D12_ROOT_SIGNATURE_FLAG_NONE);

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
                IID_PPV_ARGS(&m_rsJacobi)));
        }

        D3D12_COMPUTE_PIPELINE_STATE_DESC fieldPipelineDescCompute{};

        dx12::ComPtr<ID3DBlob> cs_field = dx12::compile_shader(
            "data/jacobi.hlsl", "cs_iterateJacobi", "cs_6_0");

        fieldPipelineDescCompute.CS.BytecodeLength = cs_field->GetBufferSize();
        fieldPipelineDescCompute.CS.pShaderBytecode =
            cs_field->GetBufferPointer();
        fieldPipelineDescCompute.pRootSignature = m_rsJacobi.Get();

        dx12::check_mhr(device->CreateComputePipelineState(
            &fieldPipelineDescCompute, IID_PPV_ARGS(&m_psoJacobi)));
    }

    // ----------------- Textures
    mAssert(scm_nbJacobiIteration % 2 == 0);
    m_pTextureResourceJacobi.resize(scm_nbJacobiTexture);

    D3D12_RESOURCE_DESC descTexture{};
    descTexture.MipLevels          = 1;
    descTexture.Width              = m_simulationWidth;
    descTexture.Height             = m_simulationHeight;
    descTexture.Flags              = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    descTexture.DepthOrArraySize   = 1;
    descTexture.SampleDesc.Count   = 1;
    descTexture.SampleDesc.Quality = 0;
    descTexture.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    descTexture.Format             = scm_formatPressure;

    auto    oHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    HRESULT hr              = device->CreateCommittedResource(
                     &oHeapProperties, D3D12_HEAP_FLAG_NONE, &descTexture,
                     D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, nullptr,
                     IID_PPV_ARGS(m_pTextureResourceJacobi[0].GetAddressOf()));
    mAssert(hr == S_OK);
    m_pTextureResourceJacobi[0]->SetName(L"Jacobi texture 0");

    hr = device->CreateCommittedResource(
        &oHeapProperties, D3D12_HEAP_FLAG_NONE, &descTexture,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
        IID_PPV_ARGS(m_pTextureResourceJacobi[1].GetAddressOf()));
    mAssert(hr == S_OK);
    m_pTextureResourceJacobi[1]->SetName(L"Jacobi texture 1");

    // views
    D3D12_SHADER_RESOURCE_VIEW_DESC descSrv = {};
    descSrv.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
    descSrv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    descSrv.Format                  = scm_formatPressure;
    descSrv.Texture2D.MipLevels     = 1;
    descSrv.Texture2D.MostDetailedMip     = 0;
    descSrv.Texture2D.ResourceMinLODClamp = 0.0f;

    D3D12_UNORDERED_ACCESS_VIEW_DESC descUav = {};
    descUav.Format                           = scm_formatPressure;
    descUav.ViewDimension                    = D3D12_UAV_DIMENSION_TEXTURE2D;

    for (mInt i = 0; i < scm_nbJacobiTexture; ++i)
    {
        m_GPUDescHdlJacobiInput[i] = m_descriptorHeap.create_srvAndGetHandle(
            m_pTextureResourceJacobi[i], descSrv);
        m_GPUDescHdlJacobiOutput[i] = m_descriptorHeap.create_uavAndGetHandle(
            m_pTextureResourceJacobi[i], descUav);
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::setup_projectionPass()
{
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;
    HRESULT res;
    // ----------------- Root signature and pipeline state
    std::vector<CD3DX12_ROOT_PARAMETER> vRootParameters;
    vRootParameters.resize(2);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vTextureRanges;
    vTextureRanges.resize(1);
    vTextureRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vOutputRanges;
    vOutputRanges.resize(1);
    vOutputRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

    vRootParameters[0].InitAsDescriptorTable(1, vTextureRanges.data());
    vRootParameters[1].InitAsDescriptorTable(1, vOutputRanges.data());

    {
        CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
        descRootSignature.Init(vRootParameters.size(), vRootParameters.data(),
                               m_samplersDescs.size(), m_samplersDescs.data(),
                               D3D12_ROOT_SIGNATURE_FLAG_NONE);

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
            IID_PPV_ARGS(&m_rsProject)));
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC fieldPipelineDescCompute{};

    dx12::ComPtr<ID3DBlob> cs_field =
        dx12::compile_shader("data/project.hlsl", "cs_project", "cs_6_0");

    fieldPipelineDescCompute.CS.BytecodeLength  = cs_field->GetBufferSize();
    fieldPipelineDescCompute.CS.pShaderBytecode = cs_field->GetBufferPointer();
    fieldPipelineDescCompute.pRootSignature     = m_rsProject.Get();

    dx12::check_mhr(device->CreateComputePipelineState(
        &fieldPipelineDescCompute, IID_PPV_ARGS(&m_psoProject)));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::setup_arrowGenerationPass()
{
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;
    HRESULT res;
    // ----------------- Root signature and pipeline state

    std::vector<CD3DX12_ROOT_PARAMETER> vRootParameters;
    vRootParameters.resize(3);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vTextureRanges;
    vTextureRanges.resize(1);
    vTextureRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vOutputRanges;
    vOutputRanges.resize(1);
    vOutputRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

    vRootParameters[0].InitAsDescriptorTable(1, vTextureRanges.data());
    vRootParameters[1].InitAsDescriptorTable(1, vOutputRanges.data());
    vRootParameters[2].InitAsConstantBufferView(0);

    {
        CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
        descRootSignature.Init(vRootParameters.size(), vRootParameters.data(),
                               m_samplersDescs.size(), m_samplersDescs.data(),
                               D3D12_ROOT_SIGNATURE_FLAG_NONE);

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
    // Create Vextex buffer
    {
        D3D12_HEAP_PROPERTIES props;
        memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
        props.Type                 = D3D12_HEAP_TYPE_DEFAULT;
        props.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        D3D12_RESOURCE_DESC desc;
        memset(&desc, 0, sizeof(D3D12_RESOURCE_DESC));
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width     = scm_arrowFieldMaxResolutionX *
                     scm_arrowFieldMaxResolutionY * sm_nbVertexPerArrows *
                     sm_sizeVertexArrow;
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
        descUAV.Buffer.NumElements               = sm_nbVertexPerArrows *
                                     scm_arrowFieldMaxResolutionX *
                                     scm_arrowFieldMaxResolutionY;

        m_GPUDescHdlOutBuffer = m_descriptorHeap.create_uavAndGetHandle(
            m_pVertexBufferArrows, descUAV);
    }
    // Create Index buffer
    {
        D3D12_HEAP_PROPERTIES props;
        memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
        props.Type                 = D3D12_HEAP_TYPE_DEFAULT;
        props.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        D3D12_RESOURCE_DESC desc;
        memset(&desc, 0, sizeof(D3D12_RESOURCE_DESC));
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width     = scm_arrowFieldMaxResolutionX *
                     scm_arrowFieldMaxResolutionY * sm_nbIndexPerArrows *
                     sm_sizeIndexArrow;
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
    {
        m_pUploadResourcesToDelete.emplace_back();

        D3D12_HEAP_PROPERTIES props;
        memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
        props.Type                 = D3D12_HEAP_TYPE_UPLOAD;
        props.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        D3D12_RESOURCE_DESC desc;
        memset(&desc, 0, sizeof(D3D12_RESOURCE_DESC));
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width     = scm_arrowFieldMaxResolutionX *
                     scm_arrowFieldMaxResolutionY * sm_nbIndexPerArrows *
                     sm_sizeIndexArrow;
        desc.Height           = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels        = 1;
        desc.Format           = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc.Count = 1;
        desc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags            = D3D12_RESOURCE_FLAG_NONE;
        if (device->CreateCommittedResource(
                &props, D3D12_HEAP_FLAG_NONE, &desc,
                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(
                    m_pUploadResourcesToDelete.back().GetAddressOf())) < 0)
            return;

        // Upload vertex/index data into a single contiguous GPU buffer
        void*       idxResource;
        D3D12_RANGE range;
        memset(&range, 0, sizeof(D3D12_RANGE));
        if (m_pUploadResourcesToDelete.back()->Map(0, &range, &idxResource) !=
            S_OK)
        {
            mLog_error("Could not map index buffer");
            return;
        }

        auto idxDest = (mU16*)idxResource;

        mU32* indices =
            new mU32[scm_arrowFieldMaxResolutionX *
                     scm_arrowFieldMaxResolutionY * sm_nbIndexPerArrows];
        for (mUInt i = 0, vIdx = 0;
             i < scm_arrowFieldMaxResolutionX * scm_arrowFieldMaxResolutionY *
                     sm_nbIndexPerArrows;
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
               scm_arrowFieldMaxResolutionX * scm_arrowFieldMaxResolutionY *
                   sm_nbIndexPerArrows * sm_sizeIndexArrow);

        m_pUploadResourcesToDelete.back()->Unmap(0, &range);

        dx12::ComPtr<ID3D12GraphicsCommandList2> pUploadCommandList =
            dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                .get_commandList();

        pUploadCommandList->CopyResource(
            m_pIndexBufferArrows.Get(),
            m_pUploadResourcesToDelete.back().Get());

        auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_pIndexBufferArrows.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_INDEX_BUFFER);
        pUploadCommandList->ResourceBarrier(1, &resourceBarrier);

        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .execute_commandList(pUploadCommandList.Get());

        delete[] indices;
    }
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
    vRootParameters.resize(1);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vTextureRanges;
    vTextureRanges.resize(1);
    vTextureRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    vRootParameters[0].InitAsDescriptorTable(1, vTextureRanges.data(),
                                             D3D12_SHADER_VISIBILITY_PIXEL);

    {
        CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
        descRootSignature.Init(vRootParameters.size(), vRootParameters.data(),
                               m_samplersDescs.size(), m_samplersDescs.data(),
                               D3D12_ROOT_SIGNATURE_FLAG_NONE);

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