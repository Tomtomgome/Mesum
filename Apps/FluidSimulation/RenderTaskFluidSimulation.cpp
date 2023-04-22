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

    // --
    sSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    hr = device->CreateDescriptorHeap(&sSrvHeapDesc,
                                      IID_PPV_ARGS(m_pHeapCPU.GetAddressOf()));
    mAssert(hr == S_OK);
    m_pHeapCPU.Get()->SetName(L"SRV Heap CPU");
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ResourceDescriptor DescriptorHeapFluidSimulation::create_srvAndGetHandle(
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

    CD3DX12_CPU_DESCRIPTOR_HANDLE const hdlCPUWrite(
        m_pHeapCPU->GetCPUDescriptorHandleForHeapStart(), m_currentHandle,
        m_incrementSizeSrv);
    device->CreateShaderResourceView(a_pResource.Get(), &a_descSrv,
                                     hdlCPUWrite);

    m_currentHandle++;

    return {hdlGPU, hdlCPUWrite};
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ResourceDescriptor DescriptorHeapFluidSimulation::create_uavAndGetHandle(
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

    CD3DX12_CPU_DESCRIPTOR_HANDLE const hdlCPUWrite(
        m_pHeapCPU->GetCPUDescriptorHandleForHeapStart(), m_currentHandle,
        m_incrementSizeSrv);
    device->CreateUnorderedAccessView(a_pResource.Get(), nullptr, &a_descUav,
                                      hdlCPUWrite);

    m_currentHandle++;

    return {hdlGPU, hdlCPUWrite};
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

    init_solverResources();

    init_constantBuffer();

    // Global Timming IDs
    m_idFullFrameQuery = get_queryID();  // ROOT HAS TO BE FIRST !

    m_idFullSimulationQuery = get_queryID();
    m_idFullRenderingQuery  = get_queryID();

    // -------------------- Root Signatures
    setup_velocityAdvectionPass();

    setup_simulationPass();

    setup_advectionPass();

    setup_divergencePass();

    setup_jacobiPass();

    setup_residualPass();

    setup_mgPasses();

    setup_projectionPass();

    setup_fluidRenderingPass();

    setup_debugDataRenderingPass();

    setup_arrowGenerationPass();

    setup_arrowRenderingPass();

    // CB description
    mUInt offset             = 0;
    m_offsetResolutionArrows = offset;
    for (mUInt i = 0; i < scm_maxVCycleDepth; ++i)
    {
        offset += scm_minimalStructSize;
        m_offsetResolutionBaseImage[i] = offset;
    }

    // Profiling
    mU64 gpuFreq;
    dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
        .get_D3D12CommandQueue()
        ->GetTimestampFrequency(&gpuFreq);
    m_ratioGPUTimestampToMs = 1000.0 / mDouble(gpuFreq);

    D3D12_QUERY_HEAP_DESC descQueryHeap{};
    descQueryHeap.Count = msc_maxQueries;
    descQueryHeap.Type  = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
    device->CreateQueryHeap(&descQueryHeap, IID_PPV_ARGS(&m_heapQuery));

    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
    auto resourceDesc =
        CD3DX12_RESOURCE_DESC::Buffer(msc_maxQueries * sizeof(mU64));
    for (mUInt i = 0; i < msc_numFrames; ++i)
    {
        if (device->CreateCommittedResource(
                &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                IID_PPV_ARGS(&m_pQueryResultsBuffers[i])) < 0)
            return;
        m_pQueryResultsBuffers[i]->SetName(L"Query buffer");
    }

    m_timers.resize(m_idCurrentQuery);

    m_timers[m_idVelocityAdvectionQuery].name = "Velocity Advection";
    m_timers[m_idSimulationQuery].name        = "Simulation";
    m_timers[m_idDivergenceQuery].name        = "Divergence";
    m_timers[m_idSolverQuery].name            = "Solver";
    m_timers[m_idResidualQuery].name          = "Residual";
    m_timers[m_idProjectionQuery].name        = "Project";
    m_timers[m_idAdvectionQuery].name         = "Advection";
    m_timers[m_idArrowGenerationQuery].name   = "Arrow Generation";
    m_timers[m_idFullSimulationQuery].name    = "Simulation Root";
    m_timers[m_idFullSimulationQuery].children.push_back(
        &m_timers[m_idVelocityAdvectionQuery]);
    m_timers[m_idFullSimulationQuery].children.push_back(
        &m_timers[m_idSimulationQuery]);
    m_timers[m_idFullSimulationQuery].children.push_back(
        &m_timers[m_idDivergenceQuery]);
    m_timers[m_idFullSimulationQuery].children.push_back(
        &m_timers[m_idSolverQuery]);
    m_timers[m_idFullSimulationQuery].children.push_back(
        &m_timers[m_idResidualQuery]);
    m_timers[m_idFullSimulationQuery].children.push_back(
        &m_timers[m_idProjectionQuery]);
    m_timers[m_idFullSimulationQuery].children.push_back(
        &m_timers[m_idAdvectionQuery]);
    m_timers[m_idFullSimulationQuery].children.push_back(
        &m_timers[m_idArrowGenerationQuery]);

    m_timers[m_idFluidRenderingQuery].name = "Fluid";
    m_timers[m_idArrowRenderingQuery].name = "Arrows";
    m_timers[m_idFullRenderingQuery].name  = "Rendering Root";
    m_timers[m_idFullRenderingQuery].children.push_back(
        &m_timers[m_idFluidRenderingQuery]);
    m_timers[m_idFullRenderingQuery].children.push_back(
        &m_timers[m_idArrowRenderingQuery]);

    m_timers[m_idFullFrameQuery].name = "Root";
    m_timers[m_idFullFrameQuery].children.push_back(
        &m_timers[m_idFullSimulationQuery]);
    m_timers[m_idFullFrameQuery].children.push_back(
        &m_timers[m_idFullRenderingQuery]);

    m_currentTimers = m_timers;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::prepare()
{
    m_frameCount++;
    m_currentFrame = m_frameCount % msc_numFrames;

    CD3DX12_RANGE readRange(0, 0);
    dx12::check_mhr(m_pQueryResultsBuffers[m_currentFrame]->Map(
        0, &readRange, &m_pQueryResultData));

    mU64* timmings = static_cast<mU64*>(m_pQueryResultData);

    for (QueryID id = 0; id < m_idCurrentQuery; ++id)
    {
        m_currentTimers[id].duration =
            (m_currentTimers[id].duration +
             ((timmings[2 * id + 1] - timmings[2 * id]) *
              m_ratioGPUTimestampToMs)) *
            scm_ratioAverage;
    }

    if (m_frameCount % 30 == 0)
    {
        m_timers = m_currentTimers;
    }

    // Pre-warm for stabilization with jacobi
    auto& parameters = static_cast<TaskDataFluidSimulation::ControlParameters&>(
        unref_safe(m_taskData.pParameters));
    if (parameters.isRunning && parameters.nbJacobiIterations >= 80 &&
        m_frameCount % 20 == 0)
    {
        parameters.nbJacobiIterations /= 2;
    }
}

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
    mInt screenWidth  = parameters.screenSize.x;
    mInt screenHeight = parameters.screenSize.y;

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

    mU8* m_pBufferData = (mU8*)(m_pConstantBuffersData[m_currentFrame]);

    math::mUIVec2& resolutionArrows = *((math::mUIVec2*)(m_pBufferData));
    resolutionArrows.x              = std::max(
                     mUInt(16), std::min(mUInt(parameters.vectorRepresentationResolution.x),
                                         scm_arrowFieldMaxResolutionX));
    resolutionArrows.y = std::max(
        mUInt(16), std::min(mUInt(parameters.vectorRepresentationResolution.y),
                            scm_arrowFieldMaxResolutionY));

    for (mUInt i = 0; i < scm_maxVCycleDepth; ++i)
    {
        mU8* pData = m_pBufferData + m_offsetResolutionBaseImage[i];

        CommonConstantBuffer& constantBuffer =
            *((CommonConstantBuffer*)(pData));
        constantBuffer.resolution =
            math::mUIVec2{m_simulationWidth / mUInt(std::pow(2U, i)),
                          m_simulationHeight / mUInt(std::pow(2U, i))};
        constantBuffer.cellSize     = math::mVec2{30.0f, 30.0f};
        constantBuffer.wallAtBottom = parameters.bottomBoundIsWall;
        constantBuffer.wallAtLeft   = parameters.leftBoundIsWall;
        constantBuffer.wallAtRight  = parameters.rightBoundIsWall;
        constantBuffer.wallAtTop    = parameters.topBoundIsWall;
    }

    D3D12_GPU_VIRTUAL_ADDRESS resolutionArrowsBuffer =
        m_pConstantBuffers[m_currentFrame]->GetGPUVirtualAddress() +
        m_offsetResolutionArrows;
    D3D12_GPU_VIRTUAL_ADDRESS resolutionBaseImageBuffer =
        m_pConstantBuffers[m_currentFrame]->GetGPUVirtualAddress() +
        m_offsetResolutionBaseImage[0];

    begin_gpuTimmer(m_idFullFrameQuery);
    begin_gpuTimmer(m_idFullSimulationQuery);

    if (parameters.isRunning)
    {
        // ---------------- Velocity advection
        {
            dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
                dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                    .get_commandList();

            {
                RAIIGPUTiming advectionTimming(computeCommandList, m_heapQuery,
                                               m_idVelocityAdvectionQuery);

                computeCommandList->SetDescriptorHeaps(1, aHeaps);
                computeCommandList->SetComputeRootSignature(
                    m_rsVelocityAdvection.Get());

                computeCommandList->SetPipelineState(
                    m_psoVelocityAdvection.Get());

                computeCommandList->SetComputeRootDescriptorTable(
                    0, m_hdlDescVelocityInput[m_iOriginal].hdlGPU);
                computeCommandList->SetComputeRootDescriptorTable(
                    1, m_hdlDescVelocityOutput[m_iComputed].hdlGPU);
                computeCommandList->SetComputeRootConstantBufferView(
                    2, resolutionBaseImageBuffer);

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

                computeCommandList->SetPipelineState(
                    m_psoVelocityStaggering.Get());

                computeCommandList->SetComputeRootDescriptorTable(
                    0, m_hdlDescVelocityInput[m_iComputed].hdlGPU);
                computeCommandList->SetComputeRootDescriptorTable(
                    1, m_hdlDescVelocityOutput[m_iOriginal].hdlGPU);

                computeCommandList->Dispatch(
                    round_up<mUInt>(nbComputeCol, m_sizeComputeGroup),
                    round_up<mUInt>(nbComputeRow, m_sizeComputeGroup), 1);
            }

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceVelocity[m_iComputed].Get(),
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            resourceBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceVelocity[m_iOriginal].Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            computeCommandList->ResourceBarrier(2, resourceBarrier);

            dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                .execute_commandList(computeCommandList);
        }

        //*
        // ---------------- Simulate forces
        {
            dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
                dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                    .get_commandList();

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                pTextureResourceOriginal,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            computeCommandList->ResourceBarrier(1, resourceBarrier);
            {
                RAIIGPUTiming timming(computeCommandList, m_heapQuery,
                                      m_idSimulationQuery);
                computeCommandList->SetDescriptorHeaps(1, aHeaps);

                // TODO : Enable dynamic switching between simulations
                computeCommandList->SetPipelineState(
                    m_psoCloudSimulation.Get());
                computeCommandList->SetComputeRootSignature(
                    m_rsSimulation.Get());

                computeCommandList->SetComputeRootDescriptorTable(
                    0, m_hdlDescTextureDisplay[m_iOriginal].hdlGPU);
                computeCommandList->SetComputeRootDescriptorTable(
                    1, m_hdlDescVelocityInput[m_iOriginal].hdlGPU);
                computeCommandList->SetComputeRootDescriptorTable(
                    2, m_hdlDescVelocityOutput[m_iComputed].hdlGPU);
                computeCommandList->SetComputeRootConstantBufferView(
                    3, resolutionBaseImageBuffer);
                computeCommandList->SetComputeRootDescriptorTable(
                    4, m_hdlDescOutputDebug.hdlGPU);

                computeCommandList->Dispatch(
                    round_up<mUInt>(nbComputeCol, m_sizeComputeGroup),
                    round_up<mUInt>(nbComputeRow, m_sizeComputeGroup), 1);
            }
            dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                .execute_commandList(computeCommandList);
        }
        //*
        // ---------------- Projection
        // ------- Compute divergence
        {
            dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
                dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                    .get_commandList();

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceVelocity[m_iComputed].Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            resourceBarrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceDivergences[0].Get(),
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            computeCommandList->ResourceBarrier(2, resourceBarrier);

            {
                RAIIGPUTiming timming(computeCommandList, m_heapQuery,
                                      m_idDivergenceQuery);

                computeCommandList->SetDescriptorHeaps(1, aHeaps);

                computeCommandList->SetPipelineState(m_psoDivergence.Get());
                computeCommandList->SetComputeRootSignature(
                    m_rsDivergence.Get());

                computeCommandList->SetComputeRootDescriptorTable(
                    0, m_hdlDescVelocityInput[m_iComputed].hdlGPU);
                computeCommandList->SetComputeRootDescriptorTable(
                    1, m_hdlDescOutputDivergences[0].hdlGPU);
                computeCommandList->SetComputeRootConstantBufferView(
                    2, resolutionBaseImageBuffer);

                computeCommandList->Dispatch(
                    round_up<mUInt>(nbComputeCol, m_sizeComputeGroup),
                    round_up<mUInt>(nbComputeRow, m_sizeComputeGroup), 1);
            }

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceDivergences[0].Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            computeCommandList->ResourceBarrier(1, resourceBarrier);

            dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                .execute_commandList(computeCommandList);
        }

        {
            begin_gpuTimmer(m_idSolverQuery);
            switch (parameters.solver)
            {
                default:
                case TaskDataFluidSimulation::ControlParameters::Solver::jacobi:
                {
                    execute_jacobi(
                        parameters.nbJacobiIterations,
                        math::mUIVec2{nbComputeCol, nbComputeRow},
                        resolutionBaseImageBuffer, m_hdlDescInputDivergences[0],
                        m_hdlDescInputPressures[0], m_hdlDescOutputPressures[0],
                        m_pTextureResourcePressures[0]);
                }
                break;
                case TaskDataFluidSimulation::ControlParameters::Solver::
                    multiGridV:
                {
                    for (mUInt i = 0; i < parameters.nbMGIterations; ++i)
                    {
                        execute_mgvITeration(0);
                    }
                }
                break;
            }
            end_gpuTimmer(m_idSolverQuery);
        }

        {
            begin_gpuTimmer(m_idResidualQuery);
            execute_residualComputation(
                math::mUIVec2{nbComputeCol, nbComputeRow},
                resolutionBaseImageBuffer, m_hdlDescInputDivergences[0],
                m_hdlDescInputPressures[0][0], m_hdlDescOutputResiduals[0],
                m_pTextureResourceResiduals[0]);
            end_gpuTimmer(m_idResidualQuery);
        }

        //*
        // ------- Pressure application
        {
            dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
                dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                    .get_commandList();

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureResourceVelocity[m_iComputed].Get(),
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            computeCommandList->ResourceBarrier(1, resourceBarrier);

            {
                RAIIGPUTiming timming(computeCommandList, m_heapQuery,
                                      m_idProjectionQuery);
                computeCommandList->SetDescriptorHeaps(1, aHeaps);

                computeCommandList->SetPipelineState(m_psoProject.Get());
                computeCommandList->SetComputeRootSignature(m_rsProject.Get());

                computeCommandList->SetComputeRootDescriptorTable(
                    0, m_hdlDescInputPressures[0][0].hdlGPU);
                computeCommandList->SetComputeRootDescriptorTable(
                    1, m_hdlDescVelocityOutput[m_iComputed].hdlGPU);
                computeCommandList->SetComputeRootConstantBufferView(
                    2, resolutionBaseImageBuffer);

                computeCommandList->Dispatch(
                    round_up<mUInt>(nbComputeCol, m_sizeComputeGroup),
                    round_up<mUInt>(nbComputeRow, m_sizeComputeGroup), 1);
            }

            dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                .execute_commandList(computeCommandList);
        }
        //*/
        // ---------------- Generate arrows part
        {
            dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
                dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
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

            {
                RAIIGPUTiming timming(computeCommandList, m_heapQuery,
                                      m_idArrowGenerationQuery);
                computeCommandList->SetDescriptorHeaps(1, aHeaps);

                computeCommandList->SetPipelineState(
                    m_psoArrowGeneration.Get());
                computeCommandList->SetComputeRootSignature(
                    m_rsArrowGeneration.Get());

                computeCommandList->SetComputeRootDescriptorTable(
                    0, m_hdlDescVelocityInput[m_iComputed].hdlGPU);
                computeCommandList->SetComputeRootDescriptorTable(
                    1, m_hdlDescOutBuffer.hdlGPU);
                computeCommandList->SetComputeRootConstantBufferView(
                    2, resolutionArrowsBuffer);

                computeCommandList->Dispatch(
                    round_up<mUInt>(parameters.vectorRepresentationResolution.x,
                                    m_sizeComputeGroup),
                    round_up<mUInt>(parameters.vectorRepresentationResolution.y,
                                    m_sizeComputeGroup),
                    1);
            }
            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pVertexBufferArrows.Get(),
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
            computeCommandList->ResourceBarrier(1, resourceBarrier);

            dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                .execute_commandList(computeCommandList);
        }
        // ---------------- Advection
        {
            dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
                dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                    .get_commandList();

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                pTextureResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            computeCommandList->ResourceBarrier(1, resourceBarrier);

            {
                RAIIGPUTiming timming(computeCommandList, m_heapQuery,
                                      m_idAdvectionQuery);
                computeCommandList->SetDescriptorHeaps(1, aHeaps);

                computeCommandList->SetPipelineState(m_psoAdvection.Get());
                computeCommandList->SetComputeRootSignature(
                    m_rsAdvection.Get());

                computeCommandList->SetComputeRootDescriptorTable(
                    0, m_hdlDescTextureDisplay[m_iOriginal].hdlGPU);
                computeCommandList->SetComputeRootDescriptorTable(
                    1, m_hdlDescVelocityInput[m_iComputed].hdlGPU);
                computeCommandList->SetComputeRootDescriptorTable(
                    2, m_hdlDescTextureCompute[m_iComputed].hdlGPU);
                computeCommandList->SetComputeRootConstantBufferView(
                    3, resolutionBaseImageBuffer);

                computeCommandList->Dispatch(
                    round_up<mUInt>(nbComputeCol, m_sizeComputeGroup),
                    round_up<mUInt>(nbComputeRow, m_sizeComputeGroup), 1);
            }

            dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                .execute_commandList(computeCommandList);
        }

        // Copies to original textures
        {
            dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
                dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
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

            dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                .execute_commandList(computeCommandList);
        }
    }

    end_gpuTimmer(m_idFullSimulationQuery);

    begin_gpuTimmer(m_idFullRenderingQuery);
    // ---------------- Renders the fluid
    if (parameters.displayFluid)
    {
        dx12::ComPtr<ID3D12GraphicsCommandList2> graphicCommandList =
            dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                .get_commandList();
        {
            RAIIGPUTiming timming(graphicCommandList, m_heapQuery,
                                  m_idFluidRenderingQuery);

            graphicCommandList->SetDescriptorHeaps(1, aHeaps);

            graphicCommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
            graphicCommandList->RSSetViewports(1, &viewport);
            graphicCommandList->RSSetScissorRects(1, &scissorRect);

            graphicCommandList->SetPipelineState(m_psoFluidRendering.Get());
            graphicCommandList->SetGraphicsRootSignature(
                m_rsFluidRendering.Get());

            dx12::ComPtr<ID3D12Device> device =
                dx12::DX12Context::gs_dx12Contexte->m_device;

            graphicCommandList->IASetPrimitiveTopology(
                D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            graphicCommandList->SetGraphicsRootDescriptorTable(
                0, m_hdlDescTextureDisplay[m_iOriginal].hdlGPU);

            graphicCommandList->DrawInstanced(3, 1, 0, 0);
        }
        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .execute_commandList(graphicCommandList.Get());
    }

    // ---------------- Renders debug data
    if (parameters.displaySimulationDebug)
    {
        dx12::ComPtr<ID3D12GraphicsCommandList2> graphicCommandList =
            dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                .get_commandList();
        {
            graphicCommandList->SetDescriptorHeaps(1, aHeaps);

            graphicCommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
            graphicCommandList->RSSetViewports(1, &viewport);
            graphicCommandList->RSSetScissorRects(1, &scissorRect);

            graphicCommandList->SetPipelineState(m_psoDataRendering4f.Get());
            graphicCommandList->SetGraphicsRootSignature(
                m_rsFluidRendering.Get());

            dx12::ComPtr<ID3D12Device> device =
                dx12::DX12Context::gs_dx12Contexte->m_device;

            graphicCommandList->IASetPrimitiveTopology(
                D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureDebug.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            graphicCommandList->ResourceBarrier(1, resourceBarrier);

            graphicCommandList->SetGraphicsRootDescriptorTable(
                0, m_hdlDescInputDebug.hdlGPU);

            graphicCommandList->DrawInstanced(3, 1, 0, 0);

            resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                m_pTextureDebug.Get(),
                D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            graphicCommandList->ResourceBarrier(1, resourceBarrier);
        }
        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .execute_commandList(graphicCommandList.Get());
    }

    if (m::mInt(parameters.debugDisplay) !=
        m::mInt(
            TaskDataFluidSimulation::ControlParameters::DebugDisplays::none))
    {
        dx12::ComPtr<ID3D12GraphicsCommandList2> graphicCommandList =
            dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                .get_commandList();
        {
            graphicCommandList->SetDescriptorHeaps(1, aHeaps);

            graphicCommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
            graphicCommandList->RSSetViewports(1, &viewport);
            graphicCommandList->RSSetScissorRects(1, &scissorRect);

            graphicCommandList->SetPipelineState(m_psoDataRendering1f.Get());
            graphicCommandList->SetGraphicsRootSignature(
                m_rsFluidRendering.Get());

            dx12::ComPtr<ID3D12Device> device =
                dx12::DX12Context::gs_dx12Contexte->m_device;

            graphicCommandList->IASetPrimitiveTopology(
                D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            switch (parameters.debugDisplay)
            {
                case TaskDataFluidSimulation::ControlParameters::DebugDisplays::
                    none:
                    mAssert(false);  // Should not end up here ?
                    break;
                case TaskDataFluidSimulation::ControlParameters::DebugDisplays::
                    pressure:
                    resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                        m_pTextureResourcePressures[0][0].Get(),
                        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                    graphicCommandList->ResourceBarrier(1, resourceBarrier);

                    graphicCommandList->SetGraphicsRootDescriptorTable(
                        0, m_hdlDescInputPressures[0][0].hdlGPU);
                    break;
                case TaskDataFluidSimulation::ControlParameters::DebugDisplays::
                    divergence:
                    resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                        m_pTextureResourceDivergences[0].Get(),
                        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                    graphicCommandList->ResourceBarrier(1, resourceBarrier);

                    graphicCommandList->SetGraphicsRootDescriptorTable(
                        0, m_hdlDescInputDivergences[0].hdlGPU);
                    break;
                case TaskDataFluidSimulation::ControlParameters::DebugDisplays::
                    residual:
                    resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                        m_pTextureResourceResiduals[0].Get(),
                        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                    graphicCommandList->ResourceBarrier(1, resourceBarrier);

                    graphicCommandList->SetGraphicsRootDescriptorTable(
                        0, m_hdlDescInputResiduals[0].hdlGPU);
                    break;
            }

            graphicCommandList->DrawInstanced(3, 1, 0, 0);

            switch (parameters.debugDisplay)
            {
                case TaskDataFluidSimulation::ControlParameters::DebugDisplays::
                    none:
                    break;
                case TaskDataFluidSimulation::ControlParameters::DebugDisplays::
                    pressure:
                    resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                        m_pTextureResourcePressures[0][0].Get(),
                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                    graphicCommandList->ResourceBarrier(1, resourceBarrier);
                    break;
                case TaskDataFluidSimulation::ControlParameters::DebugDisplays::
                    divergence:
                    resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                        m_pTextureResourceDivergences[0].Get(),
                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                    graphicCommandList->ResourceBarrier(1, resourceBarrier);
                    break;
                case TaskDataFluidSimulation::ControlParameters::DebugDisplays::
                    residual:
                    resourceBarrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
                        m_pTextureResourceResiduals[0].Get(),
                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                    graphicCommandList->ResourceBarrier(1, resourceBarrier);
                    break;
            }
        }
        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .execute_commandList(graphicCommandList.Get());
    }

    // ---------------- Render Arrow
    if (parameters.displaySpeed)
    {
        dx12::ComPtr<ID3D12GraphicsCommandList2> graphicsCommandListField =
            dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                .get_commandList();

        {
            RAIIGPUTiming timming(graphicsCommandListField, m_heapQuery,
                                  m_idArrowRenderingQuery);

            graphicsCommandListField->SetDescriptorHeaps(1, aHeaps);

            graphicsCommandListField->OMSetRenderTargets(1, &rtv, FALSE,
                                                         nullptr);
            graphicsCommandListField->RSSetViewports(1, &viewport);
            graphicsCommandListField->RSSetScissorRects(1, &scissorRect);

            graphicsCommandListField->SetPipelineState(
                m_psoArrowRendering.Get());
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
        }

        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .execute_commandList(graphicsCommandListField);
    }

    end_gpuTimmer(m_idFullRenderingQuery);
    end_gpuTimmer(m_idFullFrameQuery);

    // Resolve timming data
    {
        dx12::ComPtr<ID3D12GraphicsCommandList2> commandList =
            dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                .get_commandList();

        commandList->ResolveQueryData(
            m_heapQuery.Get(), D3D12_QUERY_TYPE_TIMESTAMP, 0,
            2 * m_idCurrentQuery, m_pQueryResultsBuffers[m_currentFrame].Get(),
            0);

        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .execute_commandList(commandList);
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

    // Linear
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

    // Point
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

    // Point with border
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

    // Linear with border
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
        descStaticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        descStaticSampler.AddressV = eDx12AddressMode;
        descStaticSampler.AddressW = eDx12AddressMode;
        descStaticSampler.BorderColor =
            D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;

        descStaticSampler.ShaderRegister = registerID++;
        descStaticSampler.RegisterSpace  = 0;
        m_samplersDescs.push_back(descStaticSampler);
    }

    // Linear wrap
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
    m_hdlDescVelocityInput[0] = m_descriptorHeap.create_srvAndGetHandle(
        m_pTextureResourceVelocity[0], descSrv);
    m_hdlDescVelocityOutput[0] = m_descriptorHeap.create_uavAndGetHandle(
        m_pTextureResourceVelocity[0], descUav);

    hr = pDevice->CreateCommittedResource(
        &oHeapProperties, D3D12_HEAP_FLAG_NONE, &descTexture,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
        IID_PPV_ARGS(m_pTextureResourceVelocity[1].GetAddressOf()));
    mAssert(hr == S_OK);
    m_pTextureResourceVelocity[1]->SetName(L"Velocity Texture 1");
    m_hdlDescVelocityInput[1] = m_descriptorHeap.create_srvAndGetHandle(
        m_pTextureResourceVelocity[1], descSrv);
    m_hdlDescVelocityOutput[1] = m_descriptorHeap.create_uavAndGetHandle(
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

        m_hdlDescTextureDisplay[i] = m_descriptorHeap.create_srvAndGetHandle(
            m_pTextureResources.back(), descShaderResourceView);
        m_hdlDescTextureCompute[i] = m_descriptorHeap.create_uavAndGetHandle(
            m_pTextureResources.back(), descUav);
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::init_solverResources()
{
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;

    for (mUInt i = 0; i < scm_maxVCycleDepth; ++i)
    {
        // Desc
        D3D12_RESOURCE_DESC descTexture{};
        descTexture.MipLevels = 1;
        descTexture.Width     = m_simulationWidth / std::pow(2, i);
        descTexture.Height    = m_simulationHeight / std::pow(2, i);
        descTexture.Flags     = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        descTexture.DepthOrArraySize   = 1;
        descTexture.SampleDesc.Count   = 1;
        descTexture.SampleDesc.Quality = 0;
        descTexture.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        descTexture.Format             = scm_format1fData;

        // views
        D3D12_SHADER_RESOURCE_VIEW_DESC descSrv = {};
        descSrv.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
        descSrv.Shader4ComponentMapping =
            D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        descSrv.Format                        = scm_format1fData;
        descSrv.Texture2D.MipLevels           = 1;
        descSrv.Texture2D.MostDetailedMip     = 0;
        descSrv.Texture2D.ResourceMinLODClamp = 0.0f;

        D3D12_UNORDERED_ACCESS_VIEW_DESC descUav = {};
        descUav.Format                           = scm_format1fData;
        descUav.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

        // Divergence
        auto oHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        HRESULT hr           = device->CreateCommittedResource(
                      &oHeapProperties, D3D12_HEAP_FLAG_NONE, &descTexture,
                      D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, nullptr,
                      IID_PPV_ARGS(m_pTextureResourceDivergences[i].GetAddressOf()));
        mAssert(hr == S_OK);
        m_pTextureResourceDivergences[i]->SetName(
            std::format(L"Divergence {}", i).c_str());

        m_hdlDescInputDivergences[i] = m_descriptorHeap.create_srvAndGetHandle(
            m_pTextureResourceDivergences[i], descSrv);
        m_hdlDescOutputDivergences[i] = m_descriptorHeap.create_uavAndGetHandle(
            m_pTextureResourceDivergences[i], descUav);

        // Pressure, duplicated for the jacobi
        hr = device->CreateCommittedResource(
            &oHeapProperties, D3D12_HEAP_FLAG_NONE, &descTexture,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, nullptr,
            IID_PPV_ARGS(m_pTextureResourcePressures[i][0].GetAddressOf()));
        mAssert(hr == S_OK);
        m_pTextureResourcePressures[i][0]->SetName(
            std::format(L"Pressure it {} - {}", i, 0).c_str());

        hr = device->CreateCommittedResource(
            &oHeapProperties, D3D12_HEAP_FLAG_NONE, &descTexture,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
            IID_PPV_ARGS(m_pTextureResourcePressures[i][1].GetAddressOf()));
        mAssert(hr == S_OK);
        m_pTextureResourcePressures[i][1]->SetName(
            std::format(L"Pressure it {} - {}", i, 1).c_str());

        for (mInt j = 0; j < scm_nbJacobiTexture; ++j)
        {
            m_hdlDescInputPressures[i][j] =
                m_descriptorHeap.create_srvAndGetHandle(
                    m_pTextureResourcePressures[i][j], descSrv);
            m_hdlDescOutputPressures[i][j] =
                m_descriptorHeap.create_uavAndGetHandle(
                    m_pTextureResourcePressures[i][j], descUav);
        }

        // Residual
        hr = device->CreateCommittedResource(
            &oHeapProperties, D3D12_HEAP_FLAG_NONE, &descTexture,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, nullptr,
            IID_PPV_ARGS(m_pTextureResourceResiduals[i].GetAddressOf()));
        mAssert(hr == S_OK);
        m_pTextureResourceResiduals[i]->SetName(
            std::format(L"Residual {}", i).c_str());

        m_hdlDescInputResiduals[i] = m_descriptorHeap.create_srvAndGetHandle(
            m_pTextureResourceResiduals[i], descSrv);
        m_hdlDescOutputResiduals[i] = m_descriptorHeap.create_uavAndGetHandle(
            m_pTextureResourceResiduals[i], descUav);
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::init_constantBuffer()
{
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;

    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    for (mUInt i = 0; i < msc_numFrames; ++i)
    {
        auto resourceDesc =
            CD3DX12_RESOURCE_DESC::Buffer(scm_maxSizeConstantBuffer);
        if (device->CreateCommittedResource(
                &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                IID_PPV_ARGS(&m_pConstantBuffers[i])) < 0)
            return;
        m_pConstantBuffers[i]->SetName(L"Constant Buffer");

        CD3DX12_RANGE readRange(0, 0);
        dx12::check_mhr(m_pConstantBuffers[i]->Map(0, &readRange,
                                                   &m_pConstantBuffersData[i]));
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

    // Timmer ID
    m_idVelocityAdvectionQuery = get_queryID();
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
    vRootParameters.resize(4);

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
    vRootParameters[3].InitAsConstantBufferView(0);

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

    // Timmer ID
    m_idAdvectionQuery = get_queryID();
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
    vRootParameters.resize(5);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vTextureRanges;
    vTextureRanges.resize(1);
    vTextureRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vTextureRangesVelocity;
    vTextureRangesVelocity.resize(1);
    vTextureRangesVelocity[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vOutputRanges;
    vOutputRanges.resize(1);
    vOutputRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

    std::vector<CD3DX12_DESCRIPTOR_RANGE> vOutputDebugRanges;
    vOutputDebugRanges.resize(1);
    vOutputDebugRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0);

    vRootParameters[0].InitAsDescriptorTable(1, vTextureRanges.data());
    vRootParameters[1].InitAsDescriptorTable(1, vTextureRangesVelocity.data());
    vRootParameters[2].InitAsDescriptorTable(1, vOutputRanges.data());
    vRootParameters[3].InitAsConstantBufferView(0);
    vRootParameters[4].InitAsDescriptorTable(1, vOutputDebugRanges.data());

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

    dx12::ComPtr<ID3DBlob> cs_fluid = dx12::compile_shader(
        "data/fluidSimulation.hlsl", "cs_simulation", "cs_6_0");

    fieldPipelineDescCompute.CS.BytecodeLength  = cs_fluid->GetBufferSize();
    fieldPipelineDescCompute.CS.pShaderBytecode = cs_fluid->GetBufferPointer();
    fieldPipelineDescCompute.pRootSignature     = m_rsSimulation.Get();

    dx12::check_mhr(device->CreateComputePipelineState(
        &fieldPipelineDescCompute, IID_PPV_ARGS(&m_psoFluidSimulation)));

    dx12::ComPtr<ID3DBlob> cs_cloud = dx12::compile_shader(
        "data/cloudSimulation.hlsl", "cs_simulation", "cs_6_0");

    fieldPipelineDescCompute.CS.BytecodeLength  = cs_cloud->GetBufferSize();
    fieldPipelineDescCompute.CS.pShaderBytecode = cs_cloud->GetBufferPointer();
    dx12::check_mhr(device->CreateComputePipelineState(
        &fieldPipelineDescCompute, IID_PPV_ARGS(&m_psoCloudSimulation)));

    // Timmer ID
    m_idSimulationQuery = get_queryID();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::setup_divergencePass()
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

        std::vector<CD3DX12_DESCRIPTOR_RANGE> vOutputRanges;
        vOutputRanges.resize(1);
        vOutputRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

        vRootParameters[0].InitAsDescriptorTable(1, vDataTextureRanges.data());
        vRootParameters[1].InitAsDescriptorTable(1, vOutputRanges.data());
        vRootParameters[2].InitAsConstantBufferView(0);

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
                IID_PPV_ARGS(&m_rsDivergence)));
        }

        D3D12_COMPUTE_PIPELINE_STATE_DESC fieldPipelineDescCompute{};

        dx12::ComPtr<ID3DBlob> cs_field = dx12::compile_shader(
            "data/divergence.hlsl", "cs_divergence", "cs_6_0");

        fieldPipelineDescCompute.CS.BytecodeLength = cs_field->GetBufferSize();
        fieldPipelineDescCompute.CS.pShaderBytecode =
            cs_field->GetBufferPointer();
        fieldPipelineDescCompute.pRootSignature = m_rsDivergence.Get();

        dx12::check_mhr(device->CreateComputePipelineState(
            &fieldPipelineDescCompute, IID_PPV_ARGS(&m_psoDivergence)));
    }

    // Timmer ID
    m_idDivergenceQuery = get_queryID();
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
        vRootParameters.resize(4);

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
        vRootParameters[3].InitAsConstantBufferView(0);

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

    // Timmer ID
    m_idSolverQuery = get_queryID();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::setup_residualPass()
{
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;
    HRESULT res;
    // ----------------- Root signature and pipeline state
    {
        std::vector<CD3DX12_ROOT_PARAMETER> vRootParameters;
        vRootParameters.resize(4);

        std::vector<CD3DX12_DESCRIPTOR_RANGE> vDivergenceTextureRanges;
        vDivergenceTextureRanges.resize(1);
        vDivergenceTextureRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0,
                                         0);

        std::vector<CD3DX12_DESCRIPTOR_RANGE> vPressureTextureRanges;
        vPressureTextureRanges.resize(1);
        vPressureTextureRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1,
                                       0);

        std::vector<CD3DX12_DESCRIPTOR_RANGE> vOutputRanges;
        vOutputRanges.resize(1);
        vOutputRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

        vRootParameters[0].InitAsDescriptorTable(
            1, vDivergenceTextureRanges.data());
        vRootParameters[1].InitAsDescriptorTable(1,
                                                 vPressureTextureRanges.data());
        vRootParameters[2].InitAsDescriptorTable(1, vOutputRanges.data());
        vRootParameters[3].InitAsConstantBufferView(0);

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
                IID_PPV_ARGS(&m_rsResidual)));
        }

        D3D12_COMPUTE_PIPELINE_STATE_DESC fieldPipelineDescCompute{};

        dx12::ComPtr<ID3DBlob> cs_field =
            dx12::compile_shader("data/residual.hlsl", "cs_residual", "cs_6_0");

        fieldPipelineDescCompute.CS.BytecodeLength = cs_field->GetBufferSize();
        fieldPipelineDescCompute.CS.pShaderBytecode =
            cs_field->GetBufferPointer();
        fieldPipelineDescCompute.pRootSignature = m_rsResidual.Get();

        dx12::check_mhr(device->CreateComputePipelineState(
            &fieldPipelineDescCompute, IID_PPV_ARGS(&m_psoResidual)));
    }

    // Timmer ID
    m_idResidualQuery = get_queryID();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::setup_mgPasses()
{
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;

    D3D12_COMPUTE_PIPELINE_STATE_DESC pipelineDescCompute{};

    // Prolongation
    dx12::ComPtr<ID3DBlob> cs_prolongation = dx12::compile_shader(
        "data/mgProlongate.hlsl", "cs_prolongate", "cs_6_0");

    pipelineDescCompute.CS.BytecodeLength = cs_prolongation->GetBufferSize();
    pipelineDescCompute.CS.pShaderBytecode =
        cs_prolongation->GetBufferPointer();
    pipelineDescCompute.pRootSignature = m_rsDivergence.Get();  // Div RS

    dx12::check_mhr(device->CreateComputePipelineState(
        &pipelineDescCompute, IID_PPV_ARGS(&m_psoProlongate)));

    // Restriction
    dx12::ComPtr<ID3DBlob> cs_restriction =
        dx12::compile_shader("data/mgRestrict.hlsl", "cs_restrict", "cs_6_0");

    pipelineDescCompute.CS.BytecodeLength  = cs_restriction->GetBufferSize();
    pipelineDescCompute.CS.pShaderBytecode = cs_restriction->GetBufferPointer();
    pipelineDescCompute.pRootSignature     = m_rsDivergence.Get();  // Div RS

    dx12::check_mhr(device->CreateComputePipelineState(
        &pipelineDescCompute, IID_PPV_ARGS(&m_psoRestrict)));
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

    // Timmer ID
    m_idProjectionQuery = get_queryID();
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

        m_hdlDescOutBuffer = m_descriptorHeap.create_uavAndGetHandle(
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

    // Timmer ID
    m_idArrowGenerationQuery = get_queryID();
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

    // Timmer ID
    m_idFluidRenderingQuery = get_queryID();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::setup_debugDataRenderingPass()
{
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;
    HRESULT res;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

    dx12::ComPtr<ID3DBlob> vs =
        dx12::compile_shader("data/displayData1f.hlsl", "vs_main", "vs_6_0");
    dx12::ComPtr<ID3DBlob> ps =
        dx12::compile_shader("data/displayData1f.hlsl", "ps_main", "ps_6_0");

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
    pipelineDesc.BlendState.RenderTarget[0].DestBlend   = D3D12_BLEND_ONE;
    pipelineDesc.BlendState.RenderTarget[0].SrcBlendAlpha =
        D3D12_BLEND_SRC_ALPHA;
    pipelineDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
    pipelineDesc.BlendState.RenderTarget[0].BlendOp        = D3D12_BLEND_OP_ADD;
    pipelineDesc.BlendState.RenderTarget[0].BlendOpAlpha   = D3D12_BLEND_OP_ADD;

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
        &pipelineDesc, IID_PPV_ARGS(&m_psoDataRendering1f)));

    dx12::ComPtr<ID3DBlob> vs4f =
        dx12::compile_shader("data/displayData4f.hlsl", "vs_main", "vs_6_0");
    dx12::ComPtr<ID3DBlob> ps4f =
        dx12::compile_shader("data/displayData4f.hlsl", "ps_main", "ps_6_0");

    pipelineDesc.VS.BytecodeLength  = vs4f->GetBufferSize();
    pipelineDesc.VS.pShaderBytecode = vs4f->GetBufferPointer();
    pipelineDesc.PS.BytecodeLength  = ps4f->GetBufferSize();
    pipelineDesc.PS.pShaderBytecode = ps4f->GetBufferPointer();

    dx12::check_mhr(device->CreateGraphicsPipelineState(
        &pipelineDesc, IID_PPV_ARGS(&m_psoDataRendering4f)));

    // Simulation Debug textures
    D3D12_RESOURCE_DESC descTexture{};
    descTexture.MipLevels          = 1;
    descTexture.Width              = m_simulationWidth;
    descTexture.Height             = m_simulationHeight;
    descTexture.Flags              = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    descTexture.DepthOrArraySize   = 1;
    descTexture.SampleDesc.Count   = 1;
    descTexture.SampleDesc.Quality = 0;
    descTexture.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    descTexture.Format             = scm_formatDataTexture;

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

    auto oHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    HRESULT hr = device->CreateCommittedResource(
        &oHeapProperties, D3D12_HEAP_FLAG_NONE, &descTexture,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr,
        IID_PPV_ARGS(m_pTextureDebug.GetAddressOf()));
    mAssert(hr == S_OK);
    m_pTextureDebug->SetName(L"Debug Texture");
    m_hdlDescInputDebug = m_descriptorHeap.create_srvAndGetHandle(
        m_pTextureDebug, descShaderResourceView);
    m_hdlDescOutputDebug =
        m_descriptorHeap.create_uavAndGetHandle(m_pTextureDebug, descUav);
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

    // Timmer ID
    m_idArrowRenderingQuery = get_queryID();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
QueryID Dx12TaskFluidSimulation::get_queryID()
{
    mAssert(2 * m_idCurrentQuery < msc_maxQueries);
    return m_idCurrentQuery++;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::begin_gpuTimmer(QueryID a_idQuery) const
{
    dx12::ComPtr<ID3D12GraphicsCommandList2> commandList =
        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .get_commandList();

    commandList->EndQuery(m_heapQuery.Get(), D3D12_QUERY_TYPE_TIMESTAMP,
                          2 * a_idQuery);

    dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
        .execute_commandList(commandList);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::end_gpuTimmer(QueryID a_idQuery) const
{
    dx12::ComPtr<ID3D12GraphicsCommandList2> commandList =
        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .get_commandList();

    commandList->EndQuery(m_heapQuery.Get(), D3D12_QUERY_TYPE_TIMESTAMP,
                          2 * a_idQuery + 1);

    dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
        .execute_commandList(commandList);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::execute_jacobi(
    m::mUInt const a_nbIterations, m::math::mUIVec2 const a_size,
    D3D12_GPU_VIRTUAL_ADDRESS const& a_resolutionConstantBuffer,
    ResourceDescriptor const&        a_target,
    ResourceDescriptor const (&a_jacobiInputs)[scm_nbJacobiTexture],
    ResourceDescriptor const (&a_jacobiOutputs)[scm_nbJacobiTexture],
    m::dx12::ComPtr<ID3D12Resource> const (
        &a_textureResourceJacobi)[scm_nbJacobiTexture],
    mBool a_clear) const
{
    ID3D12DescriptorHeap* const aHeaps[1] = {m_descriptorHeap.m_pHeap.Get()};
    D3D12_RESOURCE_BARRIER      resourceBarriers[2];

    dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .get_commandList();

    if (a_clear)
    {
        resourceBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
            a_textureResourceJacobi[0].Get(),
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        computeCommandList->ResourceBarrier(1, resourceBarriers);

        mFloat clear[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        computeCommandList->ClearUnorderedAccessViewFloat(
            a_jacobiOutputs[0].hdlGPU, a_jacobiOutputs[0].hdlCPU,
            a_textureResourceJacobi[0].Get(), clear, 0, nullptr);

        resourceBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
            a_textureResourceJacobi[0].Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        computeCommandList->ResourceBarrier(1, resourceBarriers);
    }

    computeCommandList->SetDescriptorHeaps(1, aHeaps);

    computeCommandList->SetPipelineState(m_psoJacobi.Get());
    computeCommandList->SetComputeRootSignature(m_rsJacobi.Get());

    computeCommandList->SetComputeRootDescriptorTable(0, a_target.hdlGPU);
    computeCommandList->SetComputeRootConstantBufferView(
        3, a_resolutionConstantBuffer);

    for (int i = 0; i < 2 * a_nbIterations; ++i)
    {
        computeCommandList->SetComputeRootDescriptorTable(
            1, a_jacobiInputs[i % 2].hdlGPU);
        computeCommandList->SetComputeRootDescriptorTable(
            2, a_jacobiOutputs[(i + 1) % 2].hdlGPU);

        computeCommandList->Dispatch(
            round_up<mUInt>(a_size.x, m_sizeComputeGroup),
            round_up<mUInt>(a_size.y, m_sizeComputeGroup), 1);

        resourceBarriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
            a_textureResourceJacobi[i % 2].Get(),
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        resourceBarriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
            a_textureResourceJacobi[(i + 1) % 2].Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        computeCommandList->ResourceBarrier(2, resourceBarriers);
    }

    dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
        .execute_commandList(computeCommandList);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::execute_mgvITeration(
    m::mUInt const a_itNumber) const
{
    auto parameters =
        static_cast<TaskDataFluidSimulation::ControlParameters const&>(
            unref_safe(m_taskData.pParameters));

    D3D12_GPU_VIRTUAL_ADDRESS resolutionBaseImageBuffer =
        m_pConstantBuffers[m_currentFrame]->GetGPUVirtualAddress() +
        m_offsetResolutionBaseImage[a_itNumber];
    math::mUIVec2 resolution =
        math::mUIVec2{m_simulationWidth / mUInt(std::pow(2U, a_itNumber)),
                      m_simulationHeight / mUInt(std::pow(2U, a_itNumber))};
    execute_jacobi(parameters.nbMGJacobiIterations, resolution,
                   resolutionBaseImageBuffer,
                   m_hdlDescInputDivergences[a_itNumber],
                   m_hdlDescInputPressures[a_itNumber],
                   m_hdlDescOutputPressures[a_itNumber],
                   m_pTextureResourcePressures[a_itNumber]);

    execute_residualComputation(resolution, resolutionBaseImageBuffer,
                                m_hdlDescInputDivergences[a_itNumber],
                                m_hdlDescInputPressures[a_itNumber][0],
                                m_hdlDescOutputResiduals[a_itNumber],
                                m_pTextureResourceResiduals[a_itNumber]);

    D3D12_GPU_VIRTUAL_ADDRESS halfResolutionBaseImageBuffer =
        m_pConstantBuffers[m_currentFrame]->GetGPUVirtualAddress() +
        m_offsetResolutionBaseImage[a_itNumber + 1];
    math::mUIVec2 halfResolution =
        math::mUIVec2{m_simulationWidth / mUInt(std::pow(2U, a_itNumber + 1)),
                      m_simulationHeight / mUInt(std::pow(2U, a_itNumber + 1))};

    execute_computeShader(halfResolution, halfResolutionBaseImageBuffer,
                          m_psoRestrict, m_hdlDescInputResiduals[a_itNumber],
                          m_hdlDescOutputDivergences[a_itNumber + 1],
                          m_pTextureResourceDivergences[a_itNumber + 1]);

    // Clear smaller resources
    {
        dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
            dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
                .get_commandList();

        auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_pTextureResourcePressures[a_itNumber + 1][0].Get(),
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        computeCommandList->ResourceBarrier(1, &resourceBarrier);

        mFloat clear[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        computeCommandList->ClearUnorderedAccessViewFloat(
            m_hdlDescOutputPressures[a_itNumber + 1][0].hdlGPU,
            m_hdlDescOutputPressures[a_itNumber + 1][0].hdlCPU,
            m_pTextureResourcePressures[a_itNumber + 1][0].Get(), clear, 0,
            nullptr);

        resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_pTextureResourcePressures[a_itNumber + 1][0].Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        computeCommandList->ResourceBarrier(1, &resourceBarrier);

        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .execute_commandList(computeCommandList);
    }

    if (a_itNumber + 2 >= parameters.maxMGDepth)
    {
        execute_jacobi(parameters.nbMGJacobiIterations, halfResolution,
                       halfResolutionBaseImageBuffer,
                       m_hdlDescInputDivergences[a_itNumber + 1],
                       m_hdlDescInputPressures[a_itNumber + 1],
                       m_hdlDescOutputPressures[a_itNumber + 1],
                       m_pTextureResourcePressures[a_itNumber + 1]);
    }
    else
    {
        execute_mgvITeration(a_itNumber + 1);
    }

    execute_computeShader(resolution, resolutionBaseImageBuffer,
                          m_psoProlongate,
                          m_hdlDescInputPressures[a_itNumber + 1][0],
                          m_hdlDescOutputPressures[a_itNumber][0],
                          m_pTextureResourcePressures[a_itNumber][0]);

    execute_jacobi(parameters.nbMGJacobiIterations, resolution,
                   resolutionBaseImageBuffer,
                   m_hdlDescInputDivergences[a_itNumber],
                   m_hdlDescInputPressures[a_itNumber],
                   m_hdlDescOutputPressures[a_itNumber],
                   m_pTextureResourcePressures[a_itNumber]);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::execute_residualComputation(
    m::math::mUIVec2 const           a_size,
    D3D12_GPU_VIRTUAL_ADDRESS const& a_resolutionConstantBuffer,
    ResourceDescriptor const& a_target, ResourceDescriptor const& a_solution,
    ResourceDescriptor const&              a_residual,
    m::dx12::ComPtr<ID3D12Resource> const& a_textureResourceResidual) const
{
    ID3D12DescriptorHeap* const aHeaps[1] = {m_descriptorHeap.m_pHeap.Get()};
    D3D12_RESOURCE_BARRIER      resourceBarriers[2];

    dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .get_commandList();

    auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        a_textureResourceResidual.Get(),
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    computeCommandList->ResourceBarrier(1, &resourceBarrier);

    computeCommandList->SetDescriptorHeaps(1, aHeaps);

    computeCommandList->SetPipelineState(m_psoResidual.Get());
    computeCommandList->SetComputeRootSignature(m_rsResidual.Get());

    computeCommandList->SetComputeRootDescriptorTable(0, a_target.hdlGPU);
    computeCommandList->SetComputeRootDescriptorTable(1, a_solution.hdlGPU);
    computeCommandList->SetComputeRootDescriptorTable(2, a_residual.hdlGPU);
    computeCommandList->SetComputeRootConstantBufferView(
        3, a_resolutionConstantBuffer);

    computeCommandList->Dispatch(round_up<mUInt>(a_size.x, m_sizeComputeGroup),
                                 round_up<mUInt>(a_size.y, m_sizeComputeGroup),
                                 1);

    resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        a_textureResourceResidual.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    computeCommandList->ResourceBarrier(1, &resourceBarrier);

    dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
        .execute_commandList(computeCommandList);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12TaskFluidSimulation::execute_computeShader(
    m::math::mUIVec2 const                      a_size,
    D3D12_GPU_VIRTUAL_ADDRESS const&            a_resolutionConstantBuffer,
    m::dx12::ComPtr<ID3D12PipelineState> const& a_pso,
    ResourceDescriptor const& a_input, ResourceDescriptor const& a_output,
    m::dx12::ComPtr<ID3D12Resource> const& a_pTextureResourceOutput) const
{
    ID3D12DescriptorHeap* const aHeaps[1] = {m_descriptorHeap.m_pHeap.Get()};

    dx12::ComPtr<ID3D12GraphicsCommandList2> computeCommandList =
        dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
            .get_commandList();

    auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        a_pTextureResourceOutput.Get(),
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
        D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    computeCommandList->ResourceBarrier(1, &resourceBarrier);

    computeCommandList->SetDescriptorHeaps(1, aHeaps);

    computeCommandList->SetComputeRootSignature(m_rsDivergence.Get());
    computeCommandList->SetPipelineState(a_pso.Get());

    computeCommandList->SetComputeRootDescriptorTable(0, a_input.hdlGPU);
    computeCommandList->SetComputeRootDescriptorTable(1, a_output.hdlGPU);
    computeCommandList->SetComputeRootConstantBufferView(
        2, a_resolutionConstantBuffer);

    computeCommandList->Dispatch(round_up<mUInt>(a_size.x, m_sizeComputeGroup),
                                 round_up<mUInt>(a_size.y, m_sizeComputeGroup),
                                 1);

    resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        a_pTextureResourceOutput.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    computeCommandList->ResourceBarrier(1, &resourceBarrier);

    dx12::DX12Context::gs_dx12Contexte->get_graphicsCommandQueue()
        .execute_commandList(computeCommandList);
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