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

#ifdef _M_DX12_RENDERER
#define M_DX12_RENDERER
#endif  // _M_DX12_RENDERER

#ifdef _M_VULKAN_RENDERER
#define M_VULKAN_RENDERER
#endif  // _M_VULKAN_RENDERER

#ifdef _M_DX12_DEFAULT_RENDERER
#define M_DX12_DEFAULT_RENDERER
#endif  // _M_DX12_DEFAULT_RENDERER

#ifdef _M_VULKAN_DEFAULT_RENDERER
#define M_VULKAN_DEFAULT_RENDERER
#endif  // _M_DX12_DEFAULT_RENDERER

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

#ifdef mIfDx12Enabled
#undef mIfDx12Enabled
#endif

#ifdef mIfVulkanEnabled
#undef mIfVulkanEnabled
#endif

#if (defined M_DX12_RENDERER) && (!defined M_VK_IMPLEMENTAITON)
#define mIfDx12Enabled(a_something) a_something
#else
#define mIfDx12Enabled(a_something)
#endif  // M_DX12_RENDERER

#if (defined M_VULKAN_RENDERER) && (!defined M_DX12_IMPLEMENTAITON)
#define mIfVulkanEnabled(a_something) a_something
#else
#define mIfVulkanEnabled(a_something)
#endif  // M_VULKAN_RENDERER