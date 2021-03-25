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
    }
}

#elif defined M_DX12_RENDERER
 
#include <DX12Renderer/Includes.hpp>

namespace m 
{
    namespace renderApi
    {
        using namespace dx12;

        using RenderSurface = DX12Window;
    }
}

#endif

#endif //M_CrossRenderer