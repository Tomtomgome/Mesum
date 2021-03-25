#ifndef M_GRAPHICS_COMMON
#define M_GRAPHICS_COMMON
#pragma once

#ifdef _M_DLL_DYNAMIC_LINK
#ifdef _M_EXPORT
#define MesumGraphicsApi __declspec(dllexport)
#else
#define MesumGraphicsApi __declspec(dllimport)
#endif
#else
#define MesumGraphicsApi
#endif

#if defined _M_DX12_RENDERER
#define M_DX12_RENDERER
#elif defined _M_VULKAN_RENDERER
#define M_VULKAN_RENDERER
#endif

#endif //M_CORE_COMMON