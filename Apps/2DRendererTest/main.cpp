#include <File.hpp>
#include <Math.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/DX12Renderer/DX12Context.hpp>
#include <MesumGraphics/DearImgui/MesumDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTaskDearImGui.hpp>
#include <MesumGraphics/VulkanRenderer/VulkanContext.hpp>

using namespace m;

math::RandomGenerator g_randomGenerator;

struct BasicVertex
{
    math::Vec3 position;
    math::Vec4 color;
};

template <typename tt_Vertex, typename tt_Index>
struct BufferBase
{
    UInt                  vertexBufferSize = 0;
    static constexpr UInt vertexSize       = sizeof(tt_Vertex);
    UInt                  indexBufferSize  = 0;
    static constexpr UInt indexSize        = sizeof(tt_Index);
};

template <typename tt_Vertex, typename tt_Index>
struct Dx12BufferBase : public BufferBase<tt_Vertex, tt_Index>
{
    dx12::ComPtr<ID3D12Resource> vertexBuffer = nullptr;
    dx12::ComPtr<ID3D12Resource> indexBuffer  = nullptr;
};

template <typename tt_Vertex, typename tt_Index>
struct VulkanBufferBase : public BufferBase<tt_Vertex, tt_Index>
{
    VkBuffer       vertexBuffer             = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferDeviceMemory = VK_NULL_HANDLE;
    VkBuffer       indexBuffer              = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferDeviceMemory  = VK_NULL_HANDLE;
};

template <typename tt_Vertex, typename tt_Index>
void init_buffer(VulkanBufferBase<tt_Vertex, tt_Index>& a_buffer)
{
    std::vector<tt_Vertex> vertices;
    std::vector<tt_Index>  indices;
    upload_data(a_buffer, vertices, indices);
}

template <typename tt_Vertex, typename tt_Index>
void destroy_buffer(VulkanBufferBase<tt_Vertex, tt_Index>& a_buffer)
{
    VkDevice device = vulkan::VulkanContext::get_logDevice();

    if (a_buffer.vertexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, a_buffer.vertexBuffer, nullptr);
        vkFreeMemory(device, a_buffer.vertexBufferDeviceMemory, nullptr);
    }

    if (a_buffer.indexBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, a_buffer.indexBuffer, nullptr);
        vkFreeMemory(device, a_buffer.indexBufferDeviceMemory, nullptr);
    }
}

template <typename tt_Vertex, typename tt_Index>
void upload_data(VulkanBufferBase<tt_Vertex, tt_Index>& a_buffer,
                 std::vector<tt_Vertex> const&          a_vertices,
                 std::vector<tt_Index> const&           a_indices)
{
    if (a_vertices.size() == 0 || a_indices.size() == 0)
    {
        return;
    }

    VkDevice device = vulkan::VulkanContext::get_logDevice();

    if (a_buffer.vertexBufferSize < a_vertices.size())
    {
        if (a_buffer.vertexBuffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(device, a_buffer.vertexBuffer, nullptr);
            vkFreeMemory(device, a_buffer.vertexBufferDeviceMemory, nullptr);
        }
        a_buffer.vertexBufferSize = a_vertices.size() + 4000;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size  = a_buffer.vertexSize * a_buffer.vertexBufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(device, &bufferInfo, nullptr,
                           &a_buffer.vertexBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create vertex buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, a_buffer.vertexBuffer,
                                      &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize  = memRequirements.size;
        allocInfo.memoryTypeIndex = vulkan::VulkanContext::get_memoryTypeIndex(
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(device, &allocInfo, nullptr,
                             &a_buffer.vertexBufferDeviceMemory) != VK_SUCCESS)
        {
            throw std::runtime_error(
                "failed to allocate vertex buffer memory!");
        }

        vkBindBufferMemory(device, a_buffer.vertexBuffer,
                           a_buffer.vertexBufferDeviceMemory, 0);
    }

    if (a_buffer.indexBufferSize < a_indices.size())
    {
        if (a_buffer.indexBuffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(device, a_buffer.indexBuffer, nullptr);
            vkFreeMemory(device, a_buffer.indexBufferDeviceMemory, nullptr);
        }
        a_buffer.indexBufferSize = a_indices.size() + 6000;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size        = a_buffer.indexSize * a_buffer.indexBufferSize;
        bufferInfo.usage       = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(device, &bufferInfo, nullptr,
                           &a_buffer.indexBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create index buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, a_buffer.indexBuffer,
                                      &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize  = memRequirements.size;
        allocInfo.memoryTypeIndex = vulkan::VulkanContext::get_memoryTypeIndex(
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(device, &allocInfo, nullptr,
                             &a_buffer.indexBufferDeviceMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate index buffer memory!");
        }

        vkBindBufferMemory(device, a_buffer.indexBuffer,
                           a_buffer.indexBufferDeviceMemory, 0);
    }

    // Upload vertex/index data into a single contiguous GPU buffer
    void *pVtxResource, *pIdxResource;

    vkMapMemory(device, a_buffer.vertexBufferDeviceMemory, 0,
                a_vertices.size() * a_buffer.vertexSize, 0, &pVtxResource);
    vkMapMemory(device, a_buffer.indexBufferDeviceMemory, 0,
                a_indices.size() * a_buffer.indexSize, 0, &pIdxResource);

    auto vtxDest = (tt_Vertex*)pVtxResource;
    auto idxDest = (tt_Index*)pIdxResource;

    memcpy(vtxDest, a_vertices.data(), a_vertices.size() * a_buffer.vertexSize);
    memcpy(idxDest, a_indices.data(), a_indices.size() * a_buffer.indexSize);

    VkMappedMemoryRange ranges[2];
    ranges[0]        = {};
    ranges[0].sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    ranges[0].memory = a_buffer.vertexBufferDeviceMemory;
    ranges[0].size   = VK_WHOLE_SIZE;
    ranges[1]        = {};
    ranges[1].sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    ranges[1].memory = a_buffer.indexBufferDeviceMemory;
    ranges[1].size   = VK_WHOLE_SIZE;
    vkFlushMappedMemoryRanges(device, 2, ranges);

    vkUnmapMemory(device, a_buffer.vertexBufferDeviceMemory);
    vkUnmapMemory(device, a_buffer.indexBufferDeviceMemory);
}

template <typename tt_Vertex, typename tt_Index>
void upload_data(Dx12BufferBase<tt_Vertex, tt_Index>& a_buffer,
                 std::vector<tt_Vertex> const&        a_vertices,
                 std::vector<tt_Index> const&         a_indices)
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
void record_bind(
    Dx12BufferBase<tt_Vertex, tt_Index> const&         a_buffer,
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> a_commandList)
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
    Dx12BufferBase<BasicVertex, U16>[dx12::DX12Surface::scm_numFrames];

using vulkanUploadBuffers =
    VulkanBufferBase<BasicVertex, U16>[vulkan::VulkanSurface::scm_numFrames];

template <typename tt_Vertex, typename tt_Index>
struct DataMeshBuffer
{
    std::vector<tt_Vertex> m_vertices;
    std::vector<tt_Index>  m_indices;

    void clear()
    {
        m_vertices.clear();
        m_indices.clear();
    }
};

void add_square(DataMeshBuffer<BasicVertex, U16>* a_meshBuffer,
                math::Vec2 const                  a_position)
{
    mAssert(a_meshBuffer != nullptr);

    UInt        index = a_meshBuffer->m_vertices.size();
    Float       size  = 0.01;
    BasicVertex vertex;
    vertex.color    = {1.0f, 1.0f, 1.0f, 1.0f};
    vertex.position = {a_position.x - size, a_position.y - size, 0.5f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position = {a_position.x - size, a_position.y + size, 0.5f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position = {a_position.x + size, a_position.y - size, 0.5f};
    a_meshBuffer->m_vertices.push_back(vertex);
    vertex.position = {a_position.x + size, a_position.y + size, 0.5f};
    a_meshBuffer->m_vertices.push_back(vertex);

    a_meshBuffer->m_indices.push_back(index);
    a_meshBuffer->m_indices.push_back(index + 1);
    a_meshBuffer->m_indices.push_back(index + 2);
    a_meshBuffer->m_indices.push_back(index + 3);
    a_meshBuffer->m_indices.push_back(0xFFFF);
}

struct Drawer_2D
{
    void add_square(math::Vec2 const a_position)
    {
        ::add_square(&m_meshBuffer, a_position);
    }

    void reset() { m_meshBuffer.clear(); }

    DataMeshBuffer<BasicVertex, U16> m_meshBuffer;
};

struct TaskData2dRender : public render::TaskData
{
    DataMeshBuffer<BasicVertex, U16>* m_pMeshBuffer;
    render::ISurface::HdlPtr          m_hdlOutput;

    mIfDx12Enabled(render::Task* getNew_dx12Implementation(
        render::TaskData* a_data) override);
    mIfVulkanEnabled(render::Task* getNew_vulkanImplementation(
        render::TaskData* a_data) override);
};

struct Task2dRender : public render::Task
{
    explicit Task2dRender(TaskData2dRender* a_data)
    {
        mAssert(a_data != nullptr);
        m_taskData = *a_data;
    }

    TaskData2dRender m_taskData;
};

#ifdef M_DX12_RENDERER
struct Dx12Task2dRender : public Task2dRender
{
    explicit Dx12Task2dRender(TaskData2dRender* a_data) : Task2dRender(a_data)
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
        pipelineDesc.BlendState.RenderTarget[0].SrcBlend =
            D3D12_BLEND_SRC_ALPHA;
        pipelineDesc.BlendState.RenderTarget[0].DestBlend =
            D3D12_BLEND_INV_SRC_ALPHA;
        pipelineDesc.BlendState.RenderTarget[0].SrcBlendAlpha =
            D3D12_BLEND_SRC_ALPHA;
        pipelineDesc.BlendState.RenderTarget[0].DestBlendAlpha =
            D3D12_BLEND_INV_SRC_ALPHA;
        pipelineDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        pipelineDesc.BlendState.RenderTarget[0].BlendOpAlpha =
            D3D12_BLEND_OP_ADD;

        pipelineDesc.SampleMask = 0xFFFFFFFF;

        pipelineDesc.RasterizerState.SlopeScaledDepthBias  = 0;
        pipelineDesc.RasterizerState.DepthClipEnable       = false;
        pipelineDesc.RasterizerState.MultisampleEnable     = false;
        pipelineDesc.RasterizerState.AntialiasedLineEnable = false;
        pipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        pipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        pipelineDesc.RasterizerState.FrontCounterClockwise = true;
        pipelineDesc.RasterizerState.DepthBias             = 0;
        pipelineDesc.RasterizerState.DepthBiasClamp        = 0.0;

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

    void prepare() override
    {
        DataMeshBuffer meshBuffer = *m_taskData.m_pMeshBuffer;

        auto& buffer = m_buffers[(++m_i) % dx12::DX12Surface::scm_numFrames];
        ::upload_data(buffer, meshBuffer.m_vertices, meshBuffer.m_indices);
    }

    void execute() const override
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
        graphicCommandList->DrawIndexedInstanced(meshBuffer.m_indices.size(), 1,
                                                 0, 0, 0);

        dx12::DX12Context::gs_dx12Contexte->get_commandQueue()
            .execute_commandList(graphicCommandList.Get());
    }

   private:
    UInt          m_i = 0;
    uploadBuffers m_buffers;

    dx12::ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
    dx12::ComPtr<ID3D12PipelineState> m_pso           = nullptr;
};

render::Task* TaskData2dRender::getNew_dx12Implementation(
    render::TaskData* a_data)
{
    return new Dx12Task2dRender(static_cast<TaskData2dRender*>(a_data));
}
#endif  // M_DX12_RENDERER

#ifdef M_VULKAN_RENDERER
struct VulkanTask2dRender : public Task2dRender
{
    explicit VulkanTask2dRender(TaskData2dRender* a_data) : Task2dRender(a_data)
    {
        for (auto& buffer : m_buffers) { init_buffer(buffer); }

        m_vertShaderModule = vulkan::VulkanContext::create_shaderModule(
            "data/squareShader.vs.spv");
        m_fragShaderModule = vulkan::VulkanContext::create_shaderModule(
            "data/squareShader.fs.spv");

        create_renderPassAndPipeline(1280, 720);
    }
    ~VulkanTask2dRender() override
    {
        for (auto& buffer : m_buffers) { destroy_buffer(buffer); }

        VkDevice device = vulkan::VulkanContext::get_logDevice();

        vkDestroyPipeline(device, m_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
        vkDestroyRenderPass(device, m_renderPass, nullptr);

        vkDestroyShaderModule(device, m_vertShaderModule, nullptr);
        vkDestroyShaderModule(device, m_fragShaderModule, nullptr);
    }

    void create_renderPassAndPipeline(U32 a_width, U32 a_height)
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

        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions =
            {};
        attributeDescriptions[0].binding  = 0;
        attributeDescriptions[0].location = 0;
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
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
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
        viewportState.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
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
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor =
            VK_BLEND_FACTOR_ONE;  // Optional
        colorBlendAttachment.dstColorBlendFactor =
            VK_BLEND_FACTOR_ZERO;                             // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;  // Optional
        colorBlendAttachment.srcAlphaBlendFactor =
            VK_BLEND_FACTOR_ONE;  // Optional
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
        pipelineLayoutInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
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
        color_attachment.layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments    = &color_attachment;
        VkSubpassDependency dependency = {};
        dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass          = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
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
        dynamicState.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates    = dynamicStates;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount          = 2;
        pipelineInfo.pStages             = shaderStages;
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
                                      nullptr,
                                      &m_graphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
    }

    void prepare() override
    {
        DataMeshBuffer meshBuffer = *m_taskData.m_pMeshBuffer;

        auto& buffer =
            m_buffers[(++m_i) % vulkan::VulkanSurface::scm_numFrames];
        ::upload_data(buffer, meshBuffer.m_vertices, meshBuffer.m_indices);
    }

    void execute() const override
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

   private:
    vulkanUploadBuffers m_buffers;

    VkShaderModule m_vertShaderModule;
    VkShaderModule m_fragShaderModule;

    VkPipelineLayout m_pipelineLayout;
    VkRenderPass     m_renderPass;
    VkPipeline       m_graphicsPipeline;

    UInt m_i = 0;
};

render::Task* TaskData2dRender::getNew_vulkanImplementation(
    render::TaskData* a_data)
{
    return new VulkanTask2dRender(static_cast<TaskData2dRender*>(a_data));
}
#endif  // M_VULKAN_RENDERER

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
            position.x += std::sin(time * 10.0) * 0.001f;
            position.y += std::cos(time * 10.0) * 0.001f;
        }
    }

    std::vector<math::Vec2> m_squarePositions;
};

class RendererTestApp : public m::crossPlatform::IWindowedApplication
{
    void init() override
    {
        crossPlatform::IWindowedApplication::init();
        m_iRendererDx12   = new dx12::DX12Renderer();
        m_iRendererVulkan = new vulkan::VulkanRenderer();
        m_iRendererDx12->init();
        m_iRendererVulkan->init();

        g_randomGenerator.init(0);

        // SetupDx12 Window
        m_windowDx12 = add_newWindow("Dx12 Window", 1280, 720);
        m_windowDx12->link_inputManager(&m_inputManager);
        m_hdlSurfaceDx12 = m_windowDx12->link_renderer(m_iRendererDx12);
        m_windowDx12->set_asMainWindow();

        dearImGui::init(m_windowDx12);

        render::Taskset* taskset_renderPipelineDx12 =
            m_hdlSurfaceDx12->surface->addNew_renderTaskset();

        TaskData2dRender taskData_2dRender;
        taskData_2dRender.m_hdlOutput   = m_hdlSurfaceDx12;
        taskData_2dRender.m_pMeshBuffer = &m_drawer2d.m_meshBuffer;
        taskData_2dRender.add_toTaskSet(taskset_renderPipelineDx12);

        render::TaskDataDrawDearImGui taskData_drawDearImGui;
        taskData_drawDearImGui.m_hdlOutput = m_hdlSurfaceDx12;
        taskData_drawDearImGui.add_toTaskSet(taskset_renderPipelineDx12);

        m_inputManager.attach_ToKeyEvent(
            input::KeyAction::keyPressed(input::KEY_N),
            Callback<void>(&m_bunchOfSquares, &BunchOfSquares::add_newSquare));

        // Setup vulkan window
        m_windowVulkan     = add_newWindow("Vulkan Window", 1280, 720);
        m_hdlSurfaceVulkan = m_windowVulkan->link_renderer(m_iRendererVulkan);

        render::Taskset* taskset_renderPipelineVulkan =
            m_hdlSurfaceVulkan->surface->addNew_renderTaskset();

        taskData_2dRender.m_hdlOutput   = m_hdlSurfaceVulkan;
        taskData_2dRender.m_pMeshBuffer = &m_drawer2d.m_meshBuffer;
        taskData_2dRender.add_toTaskSet(taskset_renderPipelineVulkan);

        m_inputManager.attach_ToKeyEvent(
            input::KeyAction::keyPressed(input::KEY_N),
            Callback<void>(&m_bunchOfSquares, &BunchOfSquares::add_newSquare));
    }

    void destroy() override
    {
        crossPlatform::IWindowedApplication::destroy();

        m_iRendererDx12->destroy();
        delete m_iRendererDx12;

        m_iRendererVulkan->destroy();
        delete m_iRendererVulkan;

        dearImGui::destroy();
    }

    Bool step(const Double& a_deltaTime) override
    {
        if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
        {
            return false;
        }

        m_bunchOfSquares.update(a_deltaTime);

        m_drawer2d.reset();

        for (auto position : m_bunchOfSquares.m_squarePositions)
        {
            m_drawer2d.add_square(position);
        }

        start_dearImGuiNewFrame(m_iRendererDx12);

        ImGui::NewFrame();
        ImGui::Begin("Engine");
        {
            ImGui::Text("frame time : %f", a_deltaTime);
            ImGui::Text("frame FPS : %f", 1.0 / a_deltaTime);
            ImGui::Text("nbSuqares : %llu",
                        m_bunchOfSquares.m_squarePositions.size());
        }
        ImGui::End();
        ImGui::Render();

        m_hdlSurfaceDx12->surface->render();
        if (m_hdlSurfaceVulkan->isValid)
        {
            m_hdlSurfaceVulkan->surface->render();
        }

        return true;
    }

    m::render::IRenderer*       m_iRendererDx12;
    m::render::ISurface::HdlPtr m_hdlSurfaceDx12;
    windows::IWindow*           m_windowDx12 = nullptr;

    m::render::IRenderer*       m_iRendererVulkan;
    m::render::ISurface::HdlPtr m_hdlSurfaceVulkan;
    windows::IWindow*           m_windowVulkan = nullptr;

    Drawer_2D m_drawer2d;

    BunchOfSquares      m_bunchOfSquares;
    input::InputManager m_inputManager;
};

M_EXECUTE_WINDOWED_APP(RendererTestApp)