#ifndef M_GRAPHICS_COMMON
#define M_GRAPHICS_COMMON
#pragma once

#ifdef _M_DLL_DYNAMIC_LINK
#ifdef _M_GRAPHICS_EXPORT
#define MesumGraphicsApi __declspec(dllexport)
#else
#define MesumGraphicsApi __declspec(dllimport)
#endif
#else
#define MesumGraphicsApi
#endif

#ifdef _M_ALL_RENDERER
#define M_ALL_RENDERER
#endif

#if defined _M_DX12_RENDERER
#define M_DX12_RENDERER
#elif defined _M_VULKAN_RENDERER
#define M_VULKAN_RENDERER
#endif

#if defined M_WIN32

#ifndef UNICODE
#define UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <consoleapi.h>

#ifdef max
#undef max
#endif

#elif defined M_UNIX

#endif

#endif //M_CORE_COMMON