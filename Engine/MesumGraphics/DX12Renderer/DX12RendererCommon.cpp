#include <DXGIDebug.h>
#include <d3d12sdklayers.h>
#include <d3d12shader.h>  // Shader reflection.
#include <dxcapi.h>       // Be sure to link with dxcompiler.lib.

#include <DX12RendererCommon.hpp>
#include <Kernel/Kernel_pch.hpp>
#include <iosfwd>

namespace m::dx12
{
extern const logging::mChannelID DX_RENDERER_ID = mLog_getId();

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


void set_dxgiDebugName(ComPtr<IDXGIObject> a_dxgiObject, std::string a_sName,
                       const mInt a_lineNumber, const mChar* a_file)
{
    std::stringstream sString;
    sString << a_sName << " (" << a_lineNumber << ":" << a_file << ")";
    a_dxgiObject->SetPrivateData(WKPDID_D3DDebugObjectName,
                                 mUInt(sString.str().size()),
                                 sString.str().c_str());
}

void set_d3g12DebugName(ComPtr<ID3D12Object> a_d3d12Object, std::string a_sName,
                        const mInt a_lineNumber, const mChar* a_file)
{
    std::stringstream sString;
    sString << a_sName << " (" << a_lineNumber << ":" << a_file << ")";
    a_d3d12Object->SetPrivateData(WKPDID_D3DDebugObjectName,
                                  mUInt(sString.str().size()),
                                  sString.str().c_str());
}

ComPtr<ID3DBlob> compile_shader(std::string const& a_shaderPath,
                                std::string const& a_entryPoint,
                                std::string const& a_target)
{
    ComPtr<IDxcUtils>     pUtils;
    ComPtr<IDxcCompiler3> pCompiler;
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

    ComPtr<IDxcIncludeHandler> pIncludeHandler;
    pUtils->CreateDefaultIncludeHandler(&pIncludeHandler);

    std::wstring lFilePath   = convert_string(a_shaderPath);
    std::wstring lTarget     = convert_string(a_target);
    std::wstring lentryPoint = convert_string(a_entryPoint);
    std::wstring pdbBase = lFilePath;
    std::wstring pdbFilePath = pdbBase + L"." + lentryPoint + L".pdb";


    LPCWSTR pszArgs[] = {
        lFilePath.c_str(),  // Optional shader source file name for error
        // reporting and for PIX shader source view.
        L"-E",
        lentryPoint.c_str(),  // Entry point.
        L"-T",
        lTarget.c_str(),  // Target.
        L"-Zi",           // Enable debug information (slim format)
        L"-Fd",
        pdbFilePath.c_str(),  // The file name of the pdb. This must either be
        // supplied or the autogenerated file name must be
        // used.
        L"-Qstrip_reflect",  // Strip reflection into a separate blob.
    };

    ComPtr<IDxcBlobEncoding> pSource = nullptr;
    pUtils->LoadFile(lFilePath.c_str(), nullptr, &pSource);
    if (pSource == nullptr)
    {
        mLog_errorTo(DX_RENDERER_ID, "Could not load the shader file");
        return nullptr;
    }

    DxcBuffer Source{};
    Source.Ptr  = pSource->GetBufferPointer();
    Source.Size = pSource->GetBufferSize();
    Source.Encoding =
        DXC_CP_ACP;  // Assume BOM says UTF8 or UTF16 or this is ANSI text.

    ComPtr<IDxcResult> pResults;
    pCompiler->Compile(
        &Source,                // Source buffer.
        pszArgs,                // Array of pointers to arguments.
        _countof(pszArgs),      // Number of arguments.
        pIncludeHandler.Get(),  // User-provided interface to handle
        // #include directives (optional).
        IID_PPV_ARGS(&pResults)  // Compiler output status, buffer, and errors.
    );

    ComPtr<IDxcBlobUtf8> pErrors = nullptr;
    pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
    // Note that d3dcompiler would return null if no errors or warnings are
    // present. IDxcCompiler3::Compile will always return an error buffer, but
    // its length will be zero if there are no warnings or errors.
    if (pErrors != nullptr && pErrors->GetStringLength() != 0)
        mLog_warningTo(DX_RENDERER_ID, "Shader ", a_shaderPath,
                     " : warnings and errors : ", pErrors->GetStringPointer());

    //
    // Save pdb.
    //
    ComPtr<IDxcBlob> pPDB = nullptr;
    ComPtr<IDxcBlobUtf16> pPDBName = nullptr;
    pResults->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPDB), &pPDBName);
    {
        FILE* fp = NULL;

        // Note that if you don't specify -Fd, a pdb name will be automatically generated. Use this file name to save the pdb so that PIX can find it quickly.
        _wfopen_s(&fp, pPDBName->GetStringPointer(), L"wb");
        fwrite(pPDB->GetBufferPointer(), pPDB->GetBufferSize(), 1, fp);
        fclose(fp);
    }

    HRESULT hrStatus;
    pResults->GetStatus(&hrStatus);
    if (FAILED(hrStatus))
    {
        mLog_errorTo(DX_RENDERER_ID, "Shader ", a_shaderPath,
                    " : Compilation faled");
        return nullptr;
    }

    ComPtr<ID3DBlob> pShader = nullptr;
    pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), nullptr);
    return pShader;
}

void enable_debugLayer()
{
    ComPtr<ID3D12Debug> debugInterface;
    check_mhr(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
}

void report_liveObjects()
{
    ComPtr<IDXGIDebug1> dxgiDebug;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
    {
        dxgiDebug->ReportLiveObjects(
            DXGI_DEBUG_ALL,
            DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL |
                                 DXGI_DEBUG_RLO_IGNORE_INTERNAL));
    }
}

mBool check_tearingSupport()
{
    mBool allowTearing = false;

    // Rather than create the DXGI 1.5 factory interface directly, we create the
    // DXGI 1.4 interface and query for the 1.5 interface. This is to enable the
    // graphics debugging tools which will not support the 1.5 factory interface
    // until a future update.
    ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(factory4.As(&factory5)))
        {
            if (FAILED(factory5->CheckFeatureSupport(
                    DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing,
                    sizeof(allowTearing))))
            {
                allowTearing = false;
            }
        }
    }

    return allowTearing;
}

ComPtr<IDXGIAdapter4> get_adapter(mBool a_useWarp)
{
    ComPtr<IDXGIFactory4> dxgiFactory;
    mUInt                 createFactoryFlags = 0;
#ifdef M_DEBUG
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif  // M_DEBUG

    check_mhr(
        CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    ComPtr<IDXGIAdapter1> dxgiAdapter1;
    ComPtr<IDXGIAdapter4> dxgiAdapter4;

    if (a_useWarp)
    {
        check_mhr(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
        check_mhr(dxgiAdapter1.As(&dxgiAdapter4));
    }
    else
    {
        mU64 size_maxDedicatedVideoMemory = 0;
        for (mUInt i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) !=
                         DXGI_ERROR_NOT_FOUND;
             ++i)
        {
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
            dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

            if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
                                            D3D_FEATURE_LEVEL_11_0,
                                            __uuidof(ID3D12Device), nullptr)) &&
                dxgiAdapterDesc1.DedicatedVideoMemory >
                    size_maxDedicatedVideoMemory)
            {
                size_maxDedicatedVideoMemory =
                    dxgiAdapterDesc1.DedicatedVideoMemory;
                check_mhr(dxgiAdapter1.As(&dxgiAdapter4));
            }
        }
    }
    return dxgiAdapter4;
}

ComPtr<ID3D12Device2> create_device(ComPtr<IDXGIAdapter4> a_adapter)
{
    ComPtr<ID3D12Device2> d3d12Device2;
    check_mhr(D3D12CreateDevice(a_adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                IID_PPV_ARGS(&d3d12Device2)));
    mD3D12DebugNamed(d3d12Device2, "Suplied device");

#ifdef M_DEBUG
    ComPtr<ID3D12InfoQueue> infoQueue;
    if (SUCCEEDED(d3d12Device2.As(&infoQueue)))
    {
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE);
        // Suppress whole categories of messages
        // D3D12_MESSAGE_CATEGORY list_categories[] = {};

        // Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY list_severities[] = {
            D3D12_MESSAGE_SEVERITY_INFO};

        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID list_denyIds[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,  // I'm
                                                                           // really
                                                                           // not
                                                                           // sure
                                                                           // how
                                                                           // to
                                                                           // avoid
                                                                           // this
                                                                           // message.
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,  // This warning occurs when
                                                     // using capture frame
                                                     // while graphics
                                                     // debugging.
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,  // This warning occurs
                                                       // when using capture
                                                       // frame while graphics
                                                       // debugging.
        };

        D3D12_INFO_QUEUE_FILTER filter = {};
        // filter.DenyList.NumCategories = _countof(list_categories);
        // filter.DenyList.pCategoryList = list_categories;
        filter.DenyList.NumSeverities = _countof(list_severities);
        filter.DenyList.pSeverityList = list_severities;
        // filter.DenyList.NumIDs = _countof(list_denyIds);
        // filter.DenyList.pIDList = list_denyIds;

        check_mhr(infoQueue->PushStorageFilter(&filter));
    }
#endif

    return d3d12Device2;
}

ComPtr<ID3D12CommandQueue> create_commandQueue(ComPtr<ID3D12Device2>   a_device,
                                               D3D12_COMMAND_LIST_TYPE a_type)
{
    ComPtr<ID3D12CommandQueue> d3d12CommandQueue;

    D3D12_COMMAND_QUEUE_DESC desc_commandQueue = {};
    desc_commandQueue.Type                     = a_type;
    desc_commandQueue.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc_commandQueue.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc_commandQueue.NodeMask = 0;

    check_mhr(a_device->CreateCommandQueue(&desc_commandQueue,
                                           IID_PPV_ARGS(&d3d12CommandQueue)));
    mD3D12DebugNamed(d3d12CommandQueue, "Suplied commandqueue");

    return d3d12CommandQueue;
}

ComPtr<IDXGISwapChain4> create_swapChain(
    HWND a_hWnd, ComPtr<ID3D12CommandQueue> a_commandQueue, mU32 a_width,
    mU32 a_height, mU32 a_bufferCount)
{
    ComPtr<IDXGISwapChain4> dxgiSwapChain4;
    ComPtr<IDXGIFactory4>   dxgiFactory4;
    UINT                    flags_createFactory = 0;
#ifdef M_DEBUG
    flags_createFactory = DXGI_CREATE_FACTORY_DEBUG;
#endif  // M_DEBUG

    check_mhr(
        CreateDXGIFactory2(flags_createFactory, IID_PPV_ARGS(&dxgiFactory4)));
    mDXGIDebugNamed(dxgiFactory4, "SwapChain Factory");

	// Create a descriptor for the swap chain.
    DXGI_SWAP_CHAIN_DESC1 desc_SwapChain = {};
    desc_SwapChain.Width                 = a_width;
    desc_SwapChain.Height                = a_height;
    desc_SwapChain.Format                = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc_SwapChain.Stereo                = false;
    desc_SwapChain.SampleDesc            = {1, 0};
    desc_SwapChain.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc_SwapChain.BufferCount           = a_bufferCount;
    desc_SwapChain.Scaling               = DXGI_SCALING_STRETCH;
    desc_SwapChain.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc_SwapChain.AlphaMode             = DXGI_ALPHA_MODE_UNSPECIFIED;
    desc_SwapChain.Flags =
        check_tearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u;

	// Create a swap chain for the window.
    ComPtr<IDXGISwapChain1> swapChain1;
    check_mhr(dxgiFactory4->CreateSwapChainForHwnd(a_commandQueue.Get(), a_hWnd,
                                                   &desc_SwapChain, nullptr,
                                                   nullptr, &swapChain1));
    mDXGIDebugNamed(swapChain1, "Base SwapChain");

    // Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
    // will be handled manually.
    check_mhr(
        dxgiFactory4->MakeWindowAssociation(a_hWnd, DXGI_MWA_NO_ALT_ENTER));

    check_mhr(swapChain1.As(&dxgiSwapChain4));
    mDXGIDebugNamed(dxgiSwapChain4, "Suplied SwapChain");

    return dxgiSwapChain4;
}

ComPtr<ID3D12DescriptorHeap> create_descriptorHeap(
    ComPtr<ID3D12Device2> a_device, D3D12_DESCRIPTOR_HEAP_TYPE a_type,
    mU32 a_numDescriptors, D3D12_DESCRIPTOR_HEAP_FLAGS a_flags)
{
    ComPtr<ID3D12DescriptorHeap> descriptorHeap;

    D3D12_DESCRIPTOR_HEAP_DESC desc_descriptorHeap = {};
    desc_descriptorHeap.NumDescriptors             = a_numDescriptors;
    desc_descriptorHeap.Type                       = a_type;
    desc_descriptorHeap.Flags                      = a_flags;

    check_mhr(a_device->CreateDescriptorHeap(&desc_descriptorHeap,
                                             IID_PPV_ARGS(&descriptorHeap)));
    mD3D12DebugNamed(descriptorHeap, "Suplied descriptor heap");

    return descriptorHeap;
}

ComPtr<ID3D12CommandAllocator> create_commandAllocator(
    ComPtr<ID3D12Device2> a_device, D3D12_COMMAND_LIST_TYPE a_type)
{
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    check_mhr(a_device->CreateCommandAllocator(
        a_type, IID_PPV_ARGS(&commandAllocator)));
    mD3D12DebugNamed(commandAllocator, "Suplied command allocator");

    return commandAllocator;
}

ComPtr<ID3D12GraphicsCommandList2> create_commandList(
    ComPtr<ID3D12Device2>          a_device,
    ComPtr<ID3D12CommandAllocator> a_commandAllocator,
    D3D12_COMMAND_LIST_TYPE        a_type)
{
    ComPtr<ID3D12GraphicsCommandList2> commandList;
    check_mhr(a_device->CreateCommandList(0, a_type, a_commandAllocator.Get(),
                                          nullptr, IID_PPV_ARGS(&commandList)));
    mD3D12DebugNamed(commandList, "Suplied commandList");

    check_mhr(commandList->Close());

    return commandList;
}

ComPtr<ID3D12Fence> create_fence(ComPtr<ID3D12Device2> a_device)
{
    ComPtr<ID3D12Fence> fence;

    check_mhr(
        a_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
    mD3D12DebugNamed(fence, "Suplied fence");

    return fence;
}

HANDLE create_eventHandle()
{
    HANDLE fenceEvent;

    fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    mAssert(fenceEvent != NULL);

    return fenceEvent;
}

mU64 signal_fence(ComPtr<ID3D12CommandQueue> a_commandQueue,
                 ComPtr<ID3D12Fence> a_fence, mU64& a_fenceValue)
{
    mU64 fenceValueForSignal = ++a_fenceValue;
    check_mhr(a_commandQueue->Signal(a_fence.Get(), fenceValueForSignal));

    return fenceValueForSignal;
}

void wait_fenceValue(ComPtr<ID3D12Fence> a_fence, uint64_t a_fenceValue,
                     HANDLE a_fenceEvent, std::chrono::milliseconds a_duration)
{
    if (a_fence->GetCompletedValue() < a_fenceValue)
    {
        check_mhr(a_fence->SetEventOnCompletion(a_fenceValue, a_fenceEvent));
        WaitForSingleObject(a_fenceEvent,
                            static_cast<DWORD>(a_duration.count()));
    }
}

void flush(ComPtr<ID3D12CommandQueue> a_commandQueue,
           ComPtr<ID3D12Fence> a_fence, uint64_t& a_fenceValue,
           HANDLE a_fenceEvent)
{
    uint64_t fenceValueForSignal =
        signal_fence(a_commandQueue, a_fence, a_fenceValue);
    wait_fenceValue(a_fence, fenceValueForSignal, a_fenceEvent);
}

}  // namespace m::dx12