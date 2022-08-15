#pragma once

#define M_VK_IMPLEMENTAITON
#include "../ApiAbstraction.hpp"
#undef M_VK_IMPLEMENTAITON

#include "VulkanRendererCommon.hpp"

#include <set>

namespace m::aa::vulkan
{
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
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
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
VKAPI_ATTR VkBool32 VKAPI_CALL callback_logDebugMessage(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    switch (messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            mLog_infoTo(m::vulkan::VK_RENDERER_ID,
                        "Validation layer : ", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            mLog_warningTo(m::vulkan::VK_RENDERER_ID,
                           "Validation layer : ", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            mLog_errorTo(m::vulkan::VK_RENDERER_ID,
                         "Validation layer : ", pCallbackData->pMessage);
            break;
        default: mInterrupt;
    }

    return VK_FALSE;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
VkResult create_debugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT*    pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void setup_debugUtilsMessengerCreateInfoExt(
    VkDebugUtilsMessengerCreateInfoEXT& a_createInfo)
{
    a_createInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    a_createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    a_createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    a_createInfo.pfnUserCallback = callback_logDebugMessage;
    a_createInfo.pUserData       = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void init(mAdapter& a_adapter)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void destroy(mAdapter& a_adapter)
{
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
    a_api.debugEnabled = a_initData.enableDebug;

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

    if (vkCreateInstance(&createInfo, nullptr, &a_api.internal.vulkan.instance))
    {
        throw std::runtime_error("failed to create instance !");
    }

    // Setup debug
    if (!a_initData.enableDebug)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfoDebugUtils = {};
    setup_debugUtilsMessengerCreateInfoExt(createInfoDebugUtils);

    if (create_debugUtilsMessengerEXT(
            a_api.internal.vulkan.instance, &createInfoDebugUtils, nullptr,
            &a_api.internal.vulkan.debugUtil) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger");
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void destroy(mApi& a_api)
{
    if (a_api.debugEnabled)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            a_api.internal.vulkan.instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(a_api.internal.vulkan.instance,
                 a_api.internal.vulkan.debugUtil, nullptr);
        }
    }

    vkDestroyInstance(a_api.internal.vulkan.instance, nullptr);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void enumerate_adapter(mApi& a_api, std::vector<mAdapter>& a_adapters)
{
    mU32 deviceCount = 0;
    vkEnumeratePhysicalDevices(a_api.internal.vulkan.instance, &deviceCount,
                               nullptr);

    a_adapters.reserve(deviceCount);

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(a_api.internal.vulkan.instance, &deviceCount,
                               devices.data());

    for (mUInt idDevice = 0; idDevice < deviceCount; ++idDevice)
    {
        a_adapters.push_back(a_api.create_adapter());
        // Set physical device handle
        mAdapter& rAdapter                      = a_adapters.back();
        rAdapter.init();
        rAdapter.internal.vulkan.physicalDevice = devices[idDevice];

        // Enumerate properties
        VkPhysicalDeviceProperties propertiesDevice;
        vkGetPhysicalDeviceProperties(devices[idDevice], &propertiesDevice);

        // Set properties
        mAdapter::Properties& rProperties = rAdapter.properties;
        rProperties.idVendor              = propertiesDevice.vendorID;
        rProperties.idDevice              = propertiesDevice.deviceID;
        switch (propertiesDevice.deviceType)
        {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            {
                rProperties.type = mAdapter::DeviceType::deviceIntegrated;
            }
            break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            {
                rProperties.type = mAdapter::DeviceType::deviceDescrete;
            }
            break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            {
                rProperties.type = mAdapter::DeviceType::deviceVirtual;
            }
            break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
            {
                rProperties.type = mAdapter::DeviceType::deviceCpu;
            }
            break;
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            default:
            {
                rProperties.type = mAdapter::DeviceType::deviceOther;
            }
            break;
        }

        // Enumerate extensions (Not flexible at the moment)
        // TODO : Expose Extentions data

        // Enumerate queue families
        // TODO : Expose Queue families data
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

}  // namespace m::aa::vulkan