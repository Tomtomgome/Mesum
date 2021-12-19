#ifndef M_RenderTask2DRender
#define M_RenderTask2DRender
#pragma once

#include <MesumCore/Kernel/MathTypes.hpp>
#include <MesumGraphics/RenderTask.hpp>
#include <MesumGraphics/Renderer.hpp>

#ifdef M_DX12_RENDERER
#include <MesumGraphics/DX12Renderer/DX12Context.hpp>
#endif  // M_DX12_RENDERER
#ifdef M_VULKAN_RENDERER
#include <MesumGraphics/VulkanRenderer/VulkanContext.hpp>
#endif  // M_VULKAN_RENDERER

namespace m::resource
{
struct mRequestImage;
}

namespace m::render
{
struct BasicVertex
{
    math::mVec3 position;
    math::mVec4 color;
    math::mVec2 uv;
};

template <typename tt_Vertex, typename tt_Index>
struct BufferBase
{
    mUInt                  vertexBufferSize = 0;
    static constexpr mUInt vertexSize       = sizeof(tt_Vertex);
    mUInt                  indexBufferSize  = 0;
    static constexpr mUInt indexSize        = sizeof(tt_Index);
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
        mLog_error("Could not map vertex buffer");
        return;
    }
    if (a_buffer.indexBuffer->Map(0, &range, &idxResource) != S_OK)
    {
        mLog_error("Could not map index buffer");
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
    Dx12BufferBase<BasicVertex, mU16>[dx12::DX12Surface::scm_numFrames];

using vulkanUploadBuffers =
    VulkanBufferBase<BasicVertex, mU16>[vulkan::VulkanSurface::scm_numFrames];

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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct TaskData2dRender : public TaskData
{
    struct mRange
    {
        mInt  materialID         = 0;
        mUInt indexStartLocation = 0;
        mUInt indexCount         = 0;
    };

    DataMeshBuffer<BasicVertex, mU16>* m_pMeshBuffer = nullptr;
    math::mMat4x4*                     m_pMatrix     = nullptr;
    std::vector<mRange>*               m_pRanges     = nullptr;
    ISurface::HdlPtr                   m_hdlOutput;

    mIfDx12Enabled(Task* getNew_dx12Implementation(TaskData* a_data) override);
    mIfVulkanEnabled(Task* getNew_vulkanImplementation(TaskData* a_data)
                         override);
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct Task2dRender : public Task
{
    explicit Task2dRender(TaskData2dRender* a_data);

    virtual mBool add_texture(resource::mRequestImage const& a_request) = 0;

    void prepare() override {}

    TaskData2dRender   m_taskData;
    static const mUInt sm_nbMaxMaterial = 32;
    static const mUInt sm_minimalCBSize = 256;
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
mIfDx12Enabled(struct Dx12Task2dRender : public Task2dRender
{
    explicit Dx12Task2dRender(TaskData2dRender* a_data);

    mBool add_texture(resource::mRequestImage const& a_request) override;

    void prepare() override;

    void execute() const override;

   private:
    mUInt         m_i = 0;
    uploadBuffers m_buffers;

    dx12::ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
    dx12::ComPtr<ID3D12PipelineState> m_pso           = nullptr;

    /// \todo Put a buffer per frame !!! This is bad !!
    dx12::ComPtr<ID3D12Resource> m_pCbMatrices = nullptr;
    void* m_pCbMatricesData = nullptr;
    dx12::ComPtr<ID3D12Resource> m_pCbMaterial = nullptr;
    void* m_pCbMaterialData = nullptr;
    std::vector<dx12::ComPtr<ID3D12Resource>> m_pTextureResources{};

    D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescHdlTexture{};

    D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescHdlSampler{};

    dx12::ComPtr<ID3D12DescriptorHeap> m_pSrvHeap = nullptr;
    mUInt m_incrementSizeSrv = 0;
    static const mUInt sm_sizeSrvHeap = 32;
    dx12::ComPtr<ID3D12DescriptorHeap> m_pSamplerHeap = nullptr;
    mUInt m_incrementSizeSampler = 0;
    static const mUInt sm_sizeSamplerHeap = 8;
};)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
mIfVulkanEnabled(struct VulkanTask2dRender : public Task2dRender
{
    explicit VulkanTask2dRender(TaskData2dRender* a_data);
    ~VulkanTask2dRender() override;

    mBool add_texture(resource::mRequestImage const& a_request) override;

    void create_renderPassAndPipeline(mU32 a_width, mU32 a_height);

    void prepare() override;

    void execute() const override;

   private:
    vulkanUploadBuffers m_buffers;

    VkShaderModule m_vertShaderModule;
    VkShaderModule m_fragShaderModule;

    VkDescriptorSetLayout m_cbDescriptorLayout;

    VkPipelineLayout m_pipelineLayout;
    VkRenderPass     m_renderPass;
    VkPipeline       m_graphicsPipeline;

    VkBuffer m_cbMatrices[vulkan::VulkanSurface::scm_numFrames];
    VkDeviceMemory m_cbMatricesMemory[vulkan::VulkanSurface::scm_numFrames];
    VkDescriptorPool m_descriptorPool;
    VkDescriptorSet m_cbMatricesSets[vulkan::VulkanSurface::scm_numFrames];

    VkDescriptorSetLayout m_bindlessTextureDescriptorLayout;
    static const mUInt sm_sizeDescriptorPool = 32;
    VkDescriptorPool m_textureDescriptorPool;
    VkDescriptorSet m_bindlessTextureDescriptorSet;

    std::vector<VkImage> m_pTextureImages{};
    std::vector<VkDeviceMemory> m_pTextureMemory{};

    std::vector<VkImageView> m_imageViews{};

    VkSampler textureSampler;

    mUInt m_i = 0;
};)

}  // namespace m::render

#endif  // M_RenderTaskDearImGui