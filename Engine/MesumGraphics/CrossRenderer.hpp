#ifndef M_CrossRenderer
#define M_CrossRenderer
#pragma once

#include <MesumGraphics/Common.hpp>

#ifdef M_ALL_RENDERER
#include <DX12Renderer/Includes.hpp>
#include <VulkanRenderer/Includes.hpp>
#else
#if defined M_VULKAN_RENDERER
#include <VulkanRenderer/Includes.hpp>
#elif defined M_DX12_RENDERER
#include <DX12Renderer/Includes.hpp>
#endif
#endif

#if defined M_VULKAN_RENDERER

namespace m
{
namespace renderApi
{
using namespace vulkan;

using RenderSurface = VulkanWindow;

}  // namespace renderApi
}  // namespace m

#elif defined M_DX12_RENDERER

namespace m
{
namespace renderApi
{
using namespace dx12;

using DefaultRenderer = DX12Renderer;
}  // namespace renderApi
}  // namespace m

#endif

#endif //M_CrossRenderer