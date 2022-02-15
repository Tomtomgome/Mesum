#pragma once

#define M_VK_IMPLEMENTAITON
#include "../ApiAbstraction.hpp"
#undef M_VK_IMPLEMENTAITON

namespace m::aa::vulkan
{
const std::vector<const mChar*> g_validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const mChar*> g_deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
mBool check_validationLayerSupport()
{
    mU32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const mChar* layerName : g_validationLayers)
    {
        mBool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
std::vector<const mChar*> get_requiedExtensions(mBool a_enableValidationLayers)
{
    std::vector<const mChar*> extensions;

    if (a_enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined M_WIN32
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined M_UNIX
    extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
    return extensions;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void init(mAdapter& a_adapter, m::aa::mAdapter::InitData const& a_initData)
{
    mLog_info("This is the Vulkan impl");
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void init(mApi& a_api, m::aa::mApi::InitData const& a_initData)
{
    if (a_initData.enableDebug && !check_validationLayerSupport())
    {
        throw(std::runtime_error(
            "Validation layers requested, but not available !"));
    }

    VkApplicationInfo appInfo  = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "Mesum Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "Mesum";
    appInfo.engineVersion      = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_2;
    mLog_info("This is the Vulkan impl");

    auto extensions = get_requiedExtensions(a_initData.enableDebug);
    VkInstanceCreateInfo createInfo    = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledExtensionCount   = static_cast<mU32>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount       = 0;
    if (a_initData.enableDebug)
    {
        createInfo.enabledLayerCount =
            static_cast<mU32>(g_validationLayers.size());
        createInfo.ppEnabledLayerNames = g_validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    mU32 extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

#ifdef PRINT_EXTENTIONS
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                           extensions.data());

    mLog_infoTo(VK_RENDERER_ID, "Available extensions");
    for (const auto& extension : extensions)
    {
        mLog_infoTo(VK_RENDERER_ID, extension.extensionName);
    }
#endif  // PRINT_EXTENTIONS

    if (vkCreateInstance(&createInfo, nullptr, &a_api.vulkanData.instance))
    {
        throw std::runtime_error("failed to create instance !");
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mAdapter create_adapter()
{
    mAdapter a;
    mLink_virtualMemberFunction(a, init);
    return a;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
mApi create_api()
{
    mApi api;
    api.create_adapter = create_adapter;
    mLink_virtualMemberFunction(api, init);
    return api;
}

}  // namespace m::aa::vulkan