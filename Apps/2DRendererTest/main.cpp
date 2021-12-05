#include <Kernel/File.hpp>
#include <Kernel/Math.hpp>
#include <MesumCore/Kernel/Image.hpp>
#include <MesumGraphics/CrossPlatform.hpp>
#include <MesumGraphics/DX12Renderer/DX12Context.hpp>
#include <MesumGraphics/DearImgui/MesumDearImGui.hpp>
#include <MesumGraphics/RenderTasks/RenderTask2DRender.hpp>
#include <MesumGraphics/RenderTasks/RenderTaskDearImGui.hpp>
#include <MesumGraphics/Resources/Texture.hpp>
#include <MesumGraphics/VulkanRenderer/VulkanContext.hpp>

using namespace m;

math::mXoRandomNumberGenerator g_randomGenerator(0);

void add_square(render::DataMeshBuffer<render::BasicVertex, mU16>* a_meshBuffer,
                math::mVec2 const                                  a_position)
{
    mSoftAssert(a_meshBuffer != nullptr);

    mUInt               index = a_meshBuffer->m_vertices.size();
    mFloat              size  = 10;
    render::BasicVertex vertex;
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
    void add_square(math::mVec2 const a_position)
    {
        ::add_square(&m_meshBuffer, a_position);
    }

    void reset() { m_meshBuffer.clear(); }

    render::DataMeshBuffer<render::BasicVertex, mU16> m_meshBuffer;
};

struct BunchOfSquares
{
    void add_newSquare()
    {
        math::mVec2 newPosition;
        for (int i = 0; i < 100; i++)
        {
            newPosition.x = g_randomGenerator.get_nextFloat()*1280;
            newPosition.y = g_randomGenerator.get_nextFloat()*720;
            m_squarePositions.push_back(newPosition);
        }
    }

    void update(const mDouble& a_deltaTime)
    {
        static mFloat time = 0.0;
        time += mFloat(a_deltaTime);
        for (auto& position : m_squarePositions)
        {
            position.x += std::sin(time * 10.0) * 0.001f;
            position.y += std::cos(time * 10.0) * 0.001f;
        }
    }

    std::vector<math::mVec2> m_squarePositions;
};

size_t get_dxgiBitsPerPixel(_In_ DXGI_FORMAT a_eFmt)
{
    switch (a_eFmt)
    {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT: return 128;

        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT: return 96;

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        case DXGI_FORMAT_Y416:
        case DXGI_FORMAT_Y210:
        case DXGI_FORMAT_Y216: return 64;

        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R11G11B10_FLOAT:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_AYUV:
        case DXGI_FORMAT_Y410:
        case DXGI_FORMAT_YUY2: return 32;

        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016: return 24;

        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_B5G5R5A1_UNORM:
        case DXGI_FORMAT_A8P8:
        case DXGI_FORMAT_B4G4R4A4_UNORM: return 16;

        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_420_OPAQUE:
        case DXGI_FORMAT_NV11: return 12;

        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_A8_UNORM:
        case DXGI_FORMAT_AI44:
        case DXGI_FORMAT_IA44:
        case DXGI_FORMAT_P8: return 8;

        case DXGI_FORMAT_R1_UNORM: return 1;

        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM: return 4;

        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB: return 8;

#if defined(_XBOX_ONE) && defined(_TITLE)

        case DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
        case DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT: return 32;

        case DXGI_FORMAT_D16_UNORM_S8_UINT:
        case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X16_TYPELESS_G8_UINT: return 24;

#endif  // _XBOX_ONE && _TITLE

        default: return 0;
    }
}

void get_dxgiSurfaceInfo(_In_ size_t a_stWidth, _In_ size_t a_stHeight,
                         _In_ DXGI_FORMAT  a_eFmt,
                         _Out_opt_ size_t* a_stOutNumBytes,
                         _Out_opt_ size_t* a_stOutRowBytes,
                         _Out_opt_ size_t* a_stOutNumRows)
{
    size_t stNumBytes = 0;
    size_t stRowBytes = 0;
    size_t stNumRows  = 0;

    bool   bBc     = false;
    bool   bPacked = false;
    bool   bPlanar = false;
    size_t stBpe   = 0;
    switch (a_eFmt)
    {
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            bBc   = true;
            stBpe = 8;
            break;

        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            bBc   = true;
            stBpe = 16;
            break;

        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_YUY2:
            bPacked = true;
            stBpe   = 4;
            break;

        case DXGI_FORMAT_Y210:
        case DXGI_FORMAT_Y216:
            bPacked = true;
            stBpe   = 8;
            break;

        case DXGI_FORMAT_NV12:
        case DXGI_FORMAT_420_OPAQUE:
            bPlanar = true;
            stBpe   = 2;
            break;

        case DXGI_FORMAT_P010:
        case DXGI_FORMAT_P016:
            bPlanar = true;
            stBpe   = 4;
            break;

#if defined(_XBOX_ONE) && defined(_TITLE)

        case DXGI_FORMAT_D16_UNORM_S8_UINT:
        case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
            bPlanar = true;
            stBpe   = 4;
            break;

#endif
    }

    if (bBc)
    {
        size_t stNumBlocksWide = 0;
        if (a_stWidth > 0)
        {
            stNumBlocksWide = std::max(size_t(1), (a_stWidth + 3) / 4);
        }
        size_t numBlocksHigh = 0;
        if (a_stHeight > 0)
        {
            numBlocksHigh = std::max(size_t(1), (a_stHeight + 3) / 4);
        }
        stRowBytes = stNumBlocksWide * stBpe;
        stNumRows  = numBlocksHigh;
        stNumBytes = stRowBytes * numBlocksHigh;
    }
    else if (bPacked)
    {
        stRowBytes = ((a_stWidth + 1) >> 1) * stBpe;
        stNumRows  = a_stHeight;
        stNumBytes = stRowBytes * a_stHeight;
    }
    else if (a_eFmt == DXGI_FORMAT_NV11)
    {
        stRowBytes = ((a_stWidth + 3) >> 2) * 4;
        stNumRows =
            a_stHeight * 2;  // Direct3D makes this simplifying assumption,
                             // although it is larger than the 4:1:1 data
        stNumBytes = stRowBytes * stNumRows;
    }
    else if (bPlanar)
    {
        stRowBytes = ((a_stWidth + 1) >> 1) * stBpe;
        stNumBytes =
            (stRowBytes * a_stHeight) + ((stRowBytes * a_stHeight + 1) >> 1);
        stNumRows = a_stHeight + ((a_stHeight + 1) >> 1);
    }
    else
    {
        size_t stBpp = get_dxgiBitsPerPixel(a_eFmt);
        stRowBytes   = (a_stWidth * stBpp + 7) / 8;  // round up to nearest byte
        stNumRows    = a_stHeight;
        stNumBytes   = stRowBytes * a_stHeight;
    }

    if (a_stOutNumBytes)
    {
        *a_stOutNumBytes = stNumBytes;
    }
    if (a_stOutRowBytes)
    {
        *a_stOutRowBytes = stRowBytes;
    }
    if (a_stOutNumRows)
    {
        *a_stOutNumRows = stNumRows;
    }
}

void load_texture(resource::mImage const& a_image)
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
    descTexture.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;

    ID3D12Device2* pDevice = dx12::DX12Context::gs_dx12Contexte->m_device.Get();

    ID3D12Resource* pTextureResource;
    auto    oHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    HRESULT hr              = pDevice->CreateCommittedResource(
                     &oHeapProperties, D3D12_HEAP_FLAG_NONE, &descTexture,
                     D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                     IID_PPV_ARGS(&pTextureResource));
    if (hr != S_OK)
    {
        mLog_error("Fail to create resource for texture");
        return;
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
        return;
    }

    std::vector<D3D12_SUBRESOURCE_DATA> vSubresources(descTexture.MipLevels);
    // mip level 0
    size_t stNumBytes;
    size_t stRowBytes;
    size_t stNumRows;
    get_dxgiSurfaceInfo(size_t(descTexture.Width), size_t(descTexture.Height),
                        descTexture.Format, &stNumBytes, &stRowBytes,
                        &stNumRows);
    D3D12_SUBRESOURCE_DATA& oTextureData = vSubresources[0];
    oTextureData.pData                   = a_image.data.data();
    oTextureData.SlicePitch              = stNumBytes;
    oTextureData.RowPitch                = stRowBytes;

    ID3D12GraphicsCommandList* pUploadCommandList = nullptr;

    UpdateSubresources(pUploadCommandList, pTextureResource,
                       pUploadTextureResource, 0, 0, subresourceCount,
                       vSubresources.data());

    D3D12_RESOURCE_STATES eAfterState =
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    auto oResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        pTextureResource, D3D12_RESOURCE_STATE_COPY_DEST, eAfterState);
    pUploadCommandList->ResourceBarrier(1, &oResourceBarrier);


}

class RendererTestApp : public m::crossPlatform::IWindowedApplication
{
    void init(mCmdLine const& a_cmdLine, void* a_appData) override
    {
        crossPlatform::IWindowedApplication::init(a_cmdLine, a_appData);
        m_iRendererDx12   = new dx12::DX12Renderer();
        m_iRendererVulkan = new vulkan::VulkanRenderer();
        m_iRendererDx12->init();
        m_iRendererVulkan->init();

        // SetupDx12 Window
        m_windowDx12 = add_newWindow("Dx12 Window", 1280, 720);
        m_windowDx12->link_inputManager(&m_inputManager);
        m_hdlSurfaceDx12 = m_windowDx12->link_renderer(m_iRendererDx12);

        dearImGui::init(*m_windowDx12);

        render::Taskset* taskset_renderPipelineDx12 =
            m_hdlSurfaceDx12->surface->addNew_renderTaskset();

        render::TaskData2dRender taskData_2dRender;
        taskData_2dRender.m_hdlOutput   = m_hdlSurfaceDx12;
        taskData_2dRender.m_pMeshBuffer = &m_drawer2d.m_meshBuffer;
        taskData_2dRender.add_toTaskSet(taskset_renderPipelineDx12);

        render::TaskDataDrawDearImGui taskData_drawDearImGui;
        taskData_drawDearImGui.m_hdlOutput = m_hdlSurfaceDx12;
        taskData_drawDearImGui.add_toTaskSet(taskset_renderPipelineDx12);

        m_inputManager.attach_toKeyEvent(
            input::mKeyAction::keyPressed(input::keyN),
            mCallback<void>(&m_bunchOfSquares, &BunchOfSquares::add_newSquare));

        // Setup vulkan window
        m_windowVulkan     = add_newWindow("Vulkan Window", 1280, 720);
        m_hdlSurfaceVulkan = m_windowVulkan->link_renderer(m_iRendererVulkan);

        render::Taskset* taskset_renderPipelineVulkan =
            m_hdlSurfaceVulkan->surface->addNew_renderTaskset();

//        taskData_2dRender.m_hdlOutput   = m_hdlSurfaceVulkan;
//        taskData_2dRender.m_pMeshBuffer = &m_drawer2d.m_meshBuffer;
//        taskData_2dRender.add_toTaskSet(taskset_renderPipelineVulkan);

        resource::mRequestImage request;
        request.path          = "data/textures/Test.png";
        resource::mImage image = resource::load_image(request);

        //        render::ManagerTexture managerTexture;
        //        GpuTextureBank TextureBankDx12 =
        //        managerTexture.link_renderer(rendererDx12); GpuTextureBank
        //        TextureBankVulkan =
        //        managerTexture.link_renderer(rendererVulkan);
        //
        //        HdlTexture hdl = managerTexture.create_handle();
        //
        //        managerTexture.install_metaData(hdl, image);
        //
        //        TextureBankDx12.upload(hdl);
        //        TextureBankVulkan.upload(hdl);

        m_inputManager.attach_toKeyEvent(
            input::mKeyAction::keyPressed(input::keyN),
            mCallback<void>(&m_bunchOfSquares, &BunchOfSquares::add_newSquare));
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

    mBool step(std::chrono::steady_clock::duration const& a_deltaTime) override
    {
        if (!m::crossPlatform::IWindowedApplication::step(a_deltaTime))
        {
            return false;
        }

        mDouble deltaTime = std::chrono::duration<mDouble>(a_deltaTime).count();
        m_bunchOfSquares.update(deltaTime);

        m_drawer2d.reset();

        for (auto position : m_bunchOfSquares.m_squarePositions)
        {
            m_drawer2d.add_square(position);
        }

        start_dearImGuiNewFrame(m_iRendererDx12);

        ImGui::NewFrame();
        ImGui::Begin("Engine");
        {
            ImGui::Text("frame time : %f", deltaTime);
            ImGui::Text("frame FPS : %f", 1.0 / deltaTime);
            ImGui::Text("nbSuqares : %llu",
                        m_bunchOfSquares.m_squarePositions.size());
        }
        ImGui::End();
        ImGui::Render();

        if (m_hdlSurfaceDx12->isValid)
        {
            m_hdlSurfaceDx12->surface->render();
        }
        if (m_hdlSurfaceVulkan->isValid)
        {
            m_hdlSurfaceVulkan->surface->render();
        }

        return true;
    }

    m::render::IRenderer*       m_iRendererDx12;
    m::render::ISurface::HdlPtr m_hdlSurfaceDx12;
    windows::mIWindow*          m_windowDx12 = nullptr;

    m::render::IRenderer*       m_iRendererVulkan;
    m::render::ISurface::HdlPtr m_hdlSurfaceVulkan;
    windows::mIWindow*          m_windowVulkan = nullptr;

    Drawer_2D m_drawer2d;

    BunchOfSquares               m_bunchOfSquares;
    input::mCallbackInputManager m_inputManager;
};

M_EXECUTE_WINDOWED_APP(RendererTestApp)