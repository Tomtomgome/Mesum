#pragma once

#define M_DX12_IMPLEMENTAITON
#include "../ApiAbstraction.hpp"
#undef M_DX12_IMPLEMENTAITON

namespace m::aa::dx12
{
void init(mAdapter& a_adapter, m::aa::mAdapter::InitData const& a_initData)
{
    mLog_info("This is the dx12 impl");
//    ComPtr<IDXGIFactory4> dxgiFactory;
//    mUInt                 createFactoryFlags = 0;
//#ifdef M_DEBUG
//    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
//#endif  // M_DEBUG
//
//    mBool a_useWarp = false;
//
//    check_MicrosoftHRESULT(
//        CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));
//
//    ComPtr<IDXGIAdapter1> dxgiAdapter1;
//    ComPtr<IDXGIAdapter4> dxgiAdapter4;
//
//    if (a_useWarp)
//    {
//        check_MicrosoftHRESULT(
//            dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
//        check_MicrosoftHRESULT(dxgiAdapter1.As(&dxgiAdapter4));
//    }
//    else
//    {
//        mU64 size_maxDedicatedVideoMemory = 0;
//        for (mUInt i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) !=
//                          DXGI_ERROR_NOT_FOUND;
//             ++i)
//        {
//            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
//            dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);
//
//            if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
//                SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(),
//                                            D3D_FEATURE_LEVEL_11_0,
//                                            __uuidof(ID3D12Device), nullptr)) &&
//                dxgiAdapterDesc1.DedicatedVideoMemory >
//                    size_maxDedicatedVideoMemory)
//            {
//                size_maxDedicatedVideoMemory =
//                    dxgiAdapterDesc1.DedicatedVideoMemory;
//                check_MicrosoftHRESULT(dxgiAdapter1.As(&dxgiAdapter4));
//            }
//        }
//    }
//    a_adapter.m_dx12Data = dxgiAdapter4;
}

void init(mApi& a_api, m::aa::mApi::InitData const& a_initData)
{
    mLog_info("This is the Dx12 impl");
}

mAdapter create_adapter()
{
    mAdapter a;
    mLink_virtualMemberFunction(a, init);
    return a;
}

mApi create_api()
{
    mApi api;
    api.create_adapter = create_adapter;
    mLink_virtualMemberFunction(api, init);
    return api;
}

}  // namespace m::aa::dx12