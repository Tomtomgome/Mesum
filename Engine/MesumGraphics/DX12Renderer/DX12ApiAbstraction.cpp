#pragma once

#define M_DX12_IMPLEMENTAITON
#include "../ApiAbstraction.hpp"
#undef M_DX12_IMPLEMENTAITON

namespace m::aa::dx12
{
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void init(mAdapter& a_adapter)
{
    mLog_info("This is the dx12 impl");
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void destroy(mAdapter& a_adapter)
{
    a_adapter.internal.dx12.adapter->Release();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mAdapter create_adapter()
{
    mAdapter a;
    mLink_virtualMemberFunction(a, init);
    mLink_virtualMemberFunction(a, destroy);
    return a;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void init(mApi& a_api, m::aa::mApi::InitData const& a_initData)
{
    mUInt createFactoryFlags = 0;

    if (a_initData.enableDebug)
    {
        // Debug layers
        ID3D12Debug* debugInterface;
        m::dx12::check_MicrosoftHRESULT(
            D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
        debugInterface->EnableDebugLayer();
        debugInterface->Release();

        // Factory creation flags
        createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
    }

    m::dx12::check_MicrosoftHRESULT(CreateDXGIFactory2(
        createFactoryFlags, IID_PPV_ARGS(&a_api.internal.dx12.factory)));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void destroy(mApi& a_api)
{
    if (a_api.debugEnabled)
    {
        IDXGIDebug1* dxgiDebug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
        {
            dxgiDebug->ReportLiveObjects(
                DXGI_DEBUG_ALL,
                DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL |
                                     DXGI_DEBUG_RLO_IGNORE_INTERNAL));
            dxgiDebug->Release();
        }
    }
    a_api.internal.dx12.factory->Release();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void enumerate_adapter(mApi& a_api, std::vector<mAdapter>& a_adapters)
{
    m::dx12::ComPtr<IDXGIAdapter1> dxgiAdapter1;
    for (mUInt i = 0; a_api.internal.dx12.factory->EnumAdapters1(
                          i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND;
         ++i)
    {
        a_adapters.push_back(a_api.create_adapter());
        mAdapter& rAdapter = a_adapters.back();
        rAdapter.init();

        m::dx12::check_MicrosoftHRESULT(dxgiAdapter1->QueryInterface(
            __uuidof(IDXGIAdapter4),
            (void**)(&rAdapter.internal.dx12.adapter)));

        DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
        dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

        // rAdapter.properties.idVender = dxgiAdapterDesc1.VendorId;
        // rAdapter.properties.idDevice = dxgiAdapterDesc1.DeviceId;
        if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
            SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
                                        D3D_FEATURE_LEVEL_11_0,
                                        __uuidof(ID3D12Device), nullptr)) &&
            dxgiAdapterDesc1.DedicatedVideoMemory >
                size_maxDedicatedVideoMemory)
        {
            size_maxDedicatedVideoMemory =
                dxgiAdapterDesc1.DedicatedVideoMemory;
            check_MicrosoftHRESULT(dxgiAdapter1.As(&dxgiAdapter4));
        }
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mApi create_api()
{
    mApi api;
    api.create_adapter = create_adapter;
    mLink_virtualMemberFunction(api, init);
    mLink_virtualMemberFunction(api, destroy);
    mLink_virtualMemberFunction(api, enumerate_adapter);
    return api;
}

}  // namespace m::aa::dx12