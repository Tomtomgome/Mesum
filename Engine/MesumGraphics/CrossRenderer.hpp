#ifndef M_CrossRenderer
#define M_CrossRenderer
#pragma once

#include <MesumGraphics/Common.hpp>

#if defined M_VULKAN_RENDERER

#include <VulkanRenderer/Includes.hpp>

namespace m
{
namespace renderApi
{
using namespace vulkan;

using RenderSurface = VulkanWindow;

}  // namespace renderApi
}  // namespace m

#elif defined M_DX12_RENDERER
 
#include <DX12Renderer/Includes.hpp>

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