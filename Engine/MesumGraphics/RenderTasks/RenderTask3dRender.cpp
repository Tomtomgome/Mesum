#include <RenderTask3dRender.h>

#include <array>

namespace m::render
{
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Task* TaskData3dRender::getNew_dx12Implementation(TaskData* a_data)
{
	return new Dx12Task3dRender(dynamic_cast<TaskData3dRender*>(a_data));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Task3dRender::Task3dRender(TaskData3dRender* a_data)
{
    mSoftAssert(a_data != nullptr);
    m_taskData = *a_data;
}

#ifdef M_DX12_RENDERER

using namespace DirectX;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
Dx12Task3dRender::Dx12Task3dRender(TaskData3dRender* a_data)
    : Task3dRender(a_data)
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc{};

    const D3D12_INPUT_ELEMENT_DESC inputElements[] = {
        {
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
        {
			"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,	D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		}
	};

    pipelineDesc.InputLayout.pInputElementDescs = inputElements;
    pipelineDesc.InputLayout.NumElements        = std::size(inputElements);
    pipelineDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;

	dx12::ComPtr<ID3DBlob> vs = dx12::compile_shader("../../../Apps/WorldExplorer/data/cubeShader.hlsl", "vs_main", "vs_6_0");
	dx12::ComPtr<ID3DBlob> ps =	dx12::compile_shader("../../../Apps/WorldExplorer/data/cubeShader.hlsl", "ps_main", "ps_6_0");

    pipelineDesc.VS.BytecodeLength  = vs->GetBufferSize();
    pipelineDesc.VS.pShaderBytecode = vs->GetBufferPointer();
    pipelineDesc.PS.BytecodeLength  = ps->GetBufferSize();
    pipelineDesc.PS.pShaderBytecode = ps->GetBufferPointer();

    pipelineDesc.NumRenderTargets = 1;
    pipelineDesc.RTVFormats[0]    = DXGI_FORMAT_B8G8R8A8_UNORM;
    pipelineDesc.BlendState       = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pipelineDesc.BlendState.RenderTarget[0].BlendEnable = 1;
    pipelineDesc.BlendState.RenderTarget[0].SrcBlend    = D3D12_BLEND_SRC_ALPHA;
    pipelineDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    pipelineDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
    pipelineDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    pipelineDesc.BlendState.RenderTarget[0].BlendOp      = D3D12_BLEND_OP_ADD;
    pipelineDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

    pipelineDesc.SampleMask = 0xFFFFFFFF;

    pipelineDesc.RasterizerState.SlopeScaledDepthBias  = 0;
    pipelineDesc.RasterizerState.DepthClipEnable       = false;
    pipelineDesc.RasterizerState.MultisampleEnable     = false;
    pipelineDesc.RasterizerState.AntialiasedLineEnable = false;
    pipelineDesc.RasterizerState.FillMode              = D3D12_FILL_MODE_SOLID;
    pipelineDesc.RasterizerState.CullMode              = D3D12_CULL_MODE_FRONT;
    pipelineDesc.RasterizerState.FrontCounterClockwise = true;
    pipelineDesc.RasterizerState.DepthBias             = 0;
    pipelineDesc.RasterizerState.DepthBiasClamp        = 0.0;

    pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineDesc.SampleDesc.Count      = 1;

	// A single 32-bit constant root parameter that is used by the vertex shader.
	CD3DX12_ROOT_PARAMETER rootParameters[1];
	rootParameters[0].InitAsConstants( sizeof( XMMATRIX ) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX );

    CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
    descRootSignature.Init(	_countof(rootParameters), rootParameters, 0,
							   nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
void Dx12Task3dRender::prepare()
{
	auto* meshBuffer = m_taskData.m_pMeshBuffer;

	auto& buffer = m_buffers[(++m_i) % dx12::DX12Surface::scm_numFrames];
	render::upload_data(buffer, meshBuffer->m_vertices, meshBuffer->m_indices);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void Dx12Task3dRender::execute() const
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







	graphicCommandList->SetGraphicsRoot32BitConstants( 0, sizeof( XMMATRIX ) / 4, m_taskData.m_matrix, 0 );




    graphicCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // TODO D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP dans le 2DRender ?? c qoi le mieu

    auto& buffer = m_buffers[(m_i) % dx12::DX12Surface::scm_numFrames];
    record_bind(buffer, graphicCommandList);
    graphicCommandList->DrawIndexedInstanced(m_taskData.m_pMeshBuffer->m_indices.size(), 1u, 0u, 0u, 0u);

    dx12::DX12Context::gs_dx12Contexte->get_commandQueue().execute_commandList(graphicCommandList.Get());
}

#endif  // M_DX12_RENDERER

}  // namespace m::render