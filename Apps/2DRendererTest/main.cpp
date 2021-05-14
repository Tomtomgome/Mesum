#include <imgui.h>

#include <File.hpp>
#include <Math.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/DX12Renderer/DX12Context.hpp>

using namespace m;

math::RandomGenerator g_randomGenerator;

struct BasicVertex
{
    math::Vec3 position;
    math::Vec4 color;
};

template <typename tt_Vertex, typename tt_Index>
struct bufferBase
{
    dx12::ComPtr<ID3D12Resource> vertexBuffer     = nullptr;
    UInt                         vertexBufferSize = 0;
    const UInt                   vertexSize       = sizeof(tt_Vertex);
    dx12::ComPtr<ID3D12Resource> indexBuffer      = nullptr;
    UInt                         indexBufferSize  = 0;
    const UInt                   indexSize        = sizeof(tt_Index);
};

template <typename tt_Vertex, typename tt_Index>
void uploadData(bufferBase<tt_Vertex, tt_Index>& a_buffer,
                std::vector<tt_Vertex> const&    a_vertices,
                std::vector<tt_Index> const&     a_indices)
{
    dx12::ComPtr<ID3D12Device> device =
        dx12::DX12Context::gs_dx12Contexte->m_device;

    if (a_buffer.vertexBuffer == nullptr ||
        a_buffer.vertexBufferSize < a_vertices.size())
    {
        a_buffer.vertexBuffer     = nullptr;
        a_buffer.vertexBufferSize = a_vertices.size() + 5000;
        D3D12_HEAP_PROPERTIES props;
        memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
        props.Type                 = D3D12_HEAP_TYPE_UPLOAD;
        props.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        D3D12_RESOURCE_DESC desc;
        memset(&desc, 0, sizeof(D3D12_RESOURCE_DESC));
        desc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width            = a_buffer.vertexBufferSize * a_buffer.vertexSize;
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
                IID_PPV_ARGS(&a_buffer.vertexBuffer)) < 0)
            return;
    }

    if (a_buffer.indexBuffer == nullptr ||
        a_buffer.indexBufferSize < a_indices.size())
    {
        a_buffer.indexBuffer     = nullptr;
        a_buffer.indexBufferSize = a_indices.size() + 10000;
        D3D12_HEAP_PROPERTIES props;
        memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
        props.Type                 = D3D12_HEAP_TYPE_UPLOAD;
        props.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        D3D12_RESOURCE_DESC desc;
        memset(&desc, 0, sizeof(D3D12_RESOURCE_DESC));
        desc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width            = a_buffer.indexBufferSize * a_buffer.indexSize;
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
                IID_PPV_ARGS(&a_buffer.indexBuffer)) < 0)
            return;
    }

    // Upload vertex/index data into a single contiguous GPU buffer
    void *      vtxResource, *idxResource;
    D3D12_RANGE range;
    memset(&range, 0, sizeof(D3D12_RANGE));
    if (a_buffer.vertexBuffer->Map(0, &range, &vtxResource) != S_OK)
    {
        mLOG_ERR("Could not map vertex buffer");
        return;
    }
    if (a_buffer.indexBuffer->Map(0, &range, &idxResource) != S_OK)
    {
        mLOG_ERR("Could not map index buffer");
        return;
    }

    auto vtxDest = (tt_Vertex*)vtxResource;
    auto idxDest = (tt_Index*)idxResource;

    memcpy(vtxDest, a_vertices.data(), a_vertices.size() * a_buffer.vertexSize);
    memcpy(idxDest, a_indices.data(), a_indices.size() * a_buffer.indexSize);

    a_buffer.vertexBuffer->Unmap(0, &range);
    a_buffer.indexBuffer->Unmap(0, &range);
}

template <typename tt_Vertex, typename tt_Index>
void record_bind(bufferBase<tt_Vertex, tt_Index> const& a_buffer,
                 ID3D12GraphicsCommandList*             a_commandList)
{
    D3D12_VERTEX_BUFFER_VIEW vbv;
    memset(&vbv, 0, sizeof(D3D12_VERTEX_BUFFER_VIEW));
    vbv.BufferLocation = a_buffer.vertexBuffer->GetGPUVirtualAddress();
    vbv.SizeInBytes    = a_buffer.vertexBufferSize * a_buffer.vertexSize;
    vbv.StrideInBytes  = a_buffer.vertexSize;
    a_commandList->IASetVertexBuffers(0, 1, &vbv);
    D3D12_INDEX_BUFFER_VIEW ibv;
    memset(&ibv, 0, sizeof(D3D12_INDEX_BUFFER_VIEW));
    ibv.BufferLocation = a_buffer.indexBuffer->GetGPUVirtualAddress();
    ibv.SizeInBytes    = a_buffer.indexBufferSize * a_buffer.indexSize;
    ibv.Format =
        a_buffer.indexSize == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    a_commandList->IASetIndexBuffer(&ibv);
}

using uploadBuffers =
    bufferBase<BasicVertex, U16>[dx12::DX12Surface::scm_numFrames];

struct Drawer_2Dprimitives
{
    void init()
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
        pipelineDesc.IBStripCutValue =
            D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;

        pipelineDesc.VS.BytecodeLength  = vs->GetBufferSize();
        pipelineDesc.VS.pShaderBytecode = vs->GetBufferPointer();
        pipelineDesc.PS.BytecodeLength  = ps->GetBufferSize();
        pipelineDesc.PS.pShaderBytecode = ps->GetBufferPointer();

        pipelineDesc.NumRenderTargets = 1;
        pipelineDesc.RTVFormats[0]    = DXGI_FORMAT_B8G8R8A8_UNORM;
        pipelineDesc.BlendState       = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        pipelineDesc.BlendState.RenderTarget[0].BlendEnable = 1;
        pipelineDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        pipelineDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        pipelineDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
        pipelineDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
        pipelineDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        pipelineDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

        pipelineDesc.SampleMask = 0xFFFFFFFF;

        pipelineDesc.RasterizerState.SlopeScaledDepthBias = 0;
        pipelineDesc.RasterizerState.DepthClipEnable = false;
        pipelineDesc.RasterizerState.MultisampleEnable = false;
        pipelineDesc.RasterizerState.AntialiasedLineEnable = false;
        pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        pipelineDesc.RasterizerState.FrontCounterClockwise = true;
        pipelineDesc.RasterizerState.DepthBias = 0;
        pipelineDesc.RasterizerState.DepthBiasClamp = 0.0;

        pipelineDesc.PrimitiveTopologyType =
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pipelineDesc.SampleDesc.Count = 1;

        CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
        descRootSignature.Init(
            0, nullptr, 0, nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        dx12::ComPtr<ID3DBlob> rootBlob;
        dx12::ComPtr<ID3DBlob> errorBlob;
        HRESULT                res;
        res = D3D12SerializeRootSignature(&descRootSignature,
                                          D3D_ROOT_SIGNATURE_VERSION_1,
                                          &rootBlob, &errorBlob);

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

    void reset()
    {
        m_vertices.clear();
        m_indices.clear();
    }

    void add_cube(math::Vec2 const a_position)
    {
        UInt        index = m_vertices.size();
        Float       size  = 0.01;
        BasicVertex vertex;
        vertex.color    = {1.0f, 1.0f, 1.0f, 1.0f};
        vertex.position = {a_position.x - size, a_position.y - size, 0.5f};
        m_vertices.push_back(vertex);
        vertex.position = {a_position.x - size, a_position.y + size, 0.5f};
        m_vertices.push_back(vertex);
        vertex.position = {a_position.x + size, a_position.y - size, 0.5f};
        m_vertices.push_back(vertex);
        vertex.position = {a_position.x + size, a_position.y + size, 0.5f};
        m_vertices.push_back(vertex);

        m_indices.push_back(index);
        m_indices.push_back(index + 1);
        m_indices.push_back(index + 2);
        m_indices.push_back(index + 3);
        m_indices.push_back(0xFFFF);
    }

    void draw(ID3D12GraphicsCommandList* a_commandList)
    {
        D3D12_VIEWPORT viewport = {};
        viewport.MaxDepth = 1.0f;
        viewport.Width = 1280;
        viewport.Height = 720;
        D3D12_RECT scissorRect = {};
        scissorRect.right = 1280;
        scissorRect.bottom = 720;

        a_commandList->RSSetViewports(1, &viewport);
        a_commandList->RSSetScissorRects(1, &scissorRect);

        a_commandList->SetPipelineState(m_pso.Get());
        a_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

        dx12::ComPtr<ID3D12Device> device =
            dx12::DX12Context::gs_dx12Contexte->m_device;

        a_commandList->IASetPrimitiveTopology(
            D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        static UInt i = 0;
        auto& buffer  = m_buffers[(i++) % dx12::DX12Surface::scm_numFrames];
        uploadData(buffer, m_vertices, m_indices);
        record_bind(buffer, a_commandList);
        a_commandList->DrawIndexedInstanced(m_indices.size(), 1, 0, 0, 0);
    }

    std::vector<BasicVertex> m_vertices;
    std::vector<U16>         m_indices;

    uploadBuffers m_buffers;

    dx12::ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
    dx12::ComPtr<ID3D12PipelineState> m_pso           = nullptr;
};

struct BunchOfSquares
{
    void add_newSquare()
    {
        math::Vec2 newPosition;
        newPosition.x = g_randomGenerator.get_nextFloat();
        newPosition.y = g_randomGenerator.get_nextFloat();
        m_squarePositions.push_back(newPosition);
    }

    void update(const Double& a_deltaTime)
    {
        static Float time = 0.0;
        time += Float(a_deltaTime);
        for (auto& position : m_squarePositions)
        {
            position.x += std::sin(time*10.0) * 0.001f;
            position.y += std::cos(time*10.0) * 0.001f;
        }
    }

    std::vector<math::Vec2> m_squarePositions;
};

using CallbackRecord =
    Callback<void, dx12::ComPtr<ID3D12GraphicsCommandList2> const&>;

struct NodeOutputWindow
{
    template <typename tt_Output>
    void output(tt_Output& a_output)
    {
        nextInstruction = CallbackRecord(&a_output, &tt_Output::record);
    }

    void record(dx12::ComPtr<ID3D12GraphicsCommandList2> const& a_commandList)
    {
        auto currentSurface =
            static_cast<dx12::DX12Surface*>(window->get_renderSurface());

        D3D12_CPU_DESCRIPTOR_HANDLE rtv;
        rtv = currentSurface->get_currentRtvDesc();
        a_commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
        currentSurface->add_commandListToExecute(a_commandList);

        if (Bool(nextInstruction))
        {
            nextInstruction(a_commandList);
        }
    }

    windows::IWindow* window;
    CallbackRecord    nextInstruction;
};

struct Node2dDrawer
{
    template <typename tt_Output>
    void output(tt_Output& a_output)
    {
        nextInstruction = CallbackRecord(&a_output, &tt_Output::record);
    }

    void record(dx12::ComPtr<ID3D12GraphicsCommandList2> const& a_commandList)
    {
        mHardAssert(m_drawer != nullptr);
        m_drawer->draw(a_commandList.Get());

        if (Bool(nextInstruction))
        {
            nextInstruction(a_commandList);
        }
    }

    Drawer_2Dprimitives* m_drawer;
    CallbackRecord       nextInstruction;
};

struct NodeStart
{
    template <typename tt_Output>
    void output(tt_Output& a_output)
    {
        nextInstruction = CallbackRecord(&a_output, &tt_Output::record);
    }

    void execute()
    {
        dx12::ComPtr<ID3D12GraphicsCommandList2> graphicCommandList =
            dx12::DX12Context::gs_dx12Contexte->get_commandQueue()
                .get_commandList();

        if (Bool(nextInstruction))
        {
            nextInstruction(graphicCommandList);
        }
    }

    CallbackRecord nextInstruction;
};

class RendererTestApp : public m::crossPlatform::IWindowedApplication
{
    void init() override
    {
        crossPlatform::IWindowedApplication::init();
        init_renderer(render::RendererApi::DX12);

        g_randomGenerator.init(0);

        m_mainWindow = add_newWindow("Test 2d Renderer", 1280, 720);
        m_mainWindow->link_inputManager(&m_inputManager);

        m_mainWindow->set_asMainWindow();
        m::Bool MultiViewportsEnabled = false;
        m_mainWindow->set_asImGuiWindow(MultiViewportsEnabled);
        set_processImGuiMultiViewports(MultiViewportsEnabled);

        m_outputNode.window = m_mainWindow;
        m_outputNode.output(m_drawerNode);
        m_drawerNode.m_drawer = &m_drawer;
        m_startNode.output(m_outputNode);

        m_inputManager.attach_ToKeyEvent(
            input::KeyAction::keyPressed(input::KEY_N),
            Callback<void>(&m_bunchOfSquares, &BunchOfSquares::add_newSquare));

        m_drawer.init();
    }

    void destroy() override { crossPlatform::IWindowedApplication::destroy(); }

    Bool step(const Double& a_deltaTime) override
    {
        if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
        {
            return false;
        }

        m_bunchOfSquares.update(a_deltaTime);

        m_drawer.reset();

        for (auto position : m_bunchOfSquares.m_squarePositions)
        {
            m_drawer.add_cube(position);
        }

        m_startNode.execute();

        start_dearImGuiNewFrame();

        ImGui::NewFrame();
        ImGui::Begin("Engine");
        {
            ImGui::Text("frame time : %f", a_deltaTime);
            ImGui::Text("frame FPS : %f", 1.0 / a_deltaTime);
            ImGui::Text("nbSuqares : %llu", m_bunchOfSquares.m_squarePositions.size());
        }
        ImGui::End();
        ImGui::Render();

        render();
        return true;
    }

    NodeOutputWindow m_outputNode;
    Node2dDrawer     m_drawerNode;
    NodeStart        m_startNode;

    Drawer_2Dprimitives m_drawer;
    BunchOfSquares      m_bunchOfSquares;
    windows::IWindow*   m_mainWindow = nullptr;
    input::InputManager m_inputManager;
};

M_EXECUTE_WINDOWED_APP(RendererTestApp)