#include <VulkanRendererCommon.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <cstring>
#include <optional>
#include <set>
#include <vector>

namespace m
{
namespace vulkan
{
extern const logging::mChannelID VK_RENDERER_ID = mLog_getId();

#ifdef M_DEBUG
const mBool g_enableValidationLayers = true;
#else
const mBool g_enableValidationLayers = false;
#endif

const mU32 c_vulkanVersion = VK_API_VERSION_1_3;

const std::vector<const mChar*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const mChar*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
mBool check_validationLayerSupport()
{
    mU32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const mChar* layerName : validationLayers)
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
std::vector<const mChar*> get_requiedExtensions()
{
    std::vector<const mChar*> extensions;

    if (g_enableValidationLayers)
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
void create_instance(VkInstance& a_InstaceToCreate)
{
    if (g_enableValidationLayers && !check_validationLayerSupport())
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
    appInfo.apiVersion         = VK_API_VERSION_1_3;

    auto                 extensions    = get_requiedExtensions();
    VkInstanceCreateInfo createInfo    = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledExtensionCount   = static_cast<mU32>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount       = 0;
    if (g_enableValidationLayers)
    {
        createInfo.enabledLayerCount =
            static_cast<mU32>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
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

    if (vkCreateInstance(&createInfo, nullptr, &a_InstaceToCreate))
    {
        throw std::runtime_error("failed to create instance !");
    }
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
            mLog_infoTo(VK_RENDERER_ID,
                        "Validation layer : ", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            mLog_warningTo(VK_RENDERER_ID,
                           "Validation layer : ", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            mLog_errorTo(VK_RENDERER_ID,
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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void setup_debugMessenger(VkInstance                a_instance,
                          VkDebugUtilsMessengerEXT& a_debugUtil)
{
    if (!g_enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    setup_debugUtilsMessengerCreateInfoExt(createInfo);

    if (create_debugUtilsMessengerEXT(a_instance, &createInfo, nullptr,
                                      &a_debugUtil) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger");
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void destroy_debugMessenger(VkInstance                a_instance,
                            VkDebugUtilsMessengerEXT& a_debugUtil)
{
    if (!g_enableValidationLayers)
        return;

    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        a_instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(a_instance, a_debugUtil, nullptr);
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
mBool check_deviceExtensionSupport(VkPhysicalDevice a_device)
{
    mU32 extensionCount;
    vkEnumerateDeviceExtensionProperties(a_device, nullptr, &extensionCount,
                                         nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(a_device, nullptr, &extensionCount,
                                         availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                             deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
mBool check_deviceSuitable(VkPhysicalDevice device)
{
    mBool extensionsSupported = check_deviceExtensionSupport(device);
    // [TODO] Will do queue families support check later
    // [TODO] Will do swap chain compabilities check later
    return extensionsSupported;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void select_physicalDevice(VkInstance        a_instance,
                           VkPhysicalDevice& a_physicalDevice)
{
    mU32 deviceCount = 0;
    vkEnumeratePhysicalDevices(a_instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support !");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(a_instance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        if (check_deviceSuitable(device))
        {
            a_physicalDevice = device;
            break;
        }
    }

    if (a_physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU !");
    }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
mBool find_graphicQueueFamilyIndex(VkPhysicalDevice a_physicalDevice,
                                   mU32&            a_queueFamilyIndex)
{
    mU32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(a_physicalDevice,
                                             &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        a_physicalDevice, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
        // Check for presentation support
#if defined M_WIN32
            vkGetPhysicalDeviceWin32PresentationSupportKHR(a_physicalDevice, i)
#elif defined M_UNIX
            vkGetPhysicalDeviceXcbPresentationSupportKHR(a_physicalDevice, i)
#endif
        )
        {
            a_queueFamilyIndex = i;
            return true;
        }
        i++;
    }

    return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void create_logicalDevice(VkPhysicalDevice a_physicalDevice,
                          VkDevice& a_logicalDevice, VkQueue& a_queue,
                          mU32& a_queueFamilyIndex)
{
    find_graphicQueueFamilyIndex(a_physicalDevice, a_queueFamilyIndex);

    float                   queuePriority   = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = a_queueFamilyIndex;
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    // Timeline semaphores
    VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeature{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
        .timelineSemaphore = VK_TRUE,
    };

    // Dynamic rendering support
    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
        .dynamicRendering = VK_TRUE,
    };

    // Bindless textures
    VkPhysicalDeviceDescriptorIndexingFeatures dstIndexingFeatures{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
        nullptr};
    VkPhysicalDeviceFeatures2 dstDeviceFeatures{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &dstIndexingFeatures};

    vkGetPhysicalDeviceFeatures2(a_physicalDevice, &dstDeviceFeatures);

    bool bindless_supported =
        dstIndexingFeatures.descriptorBindingPartiallyBound &&
        dstIndexingFeatures.runtimeDescriptorArray;

    VkPhysicalDeviceFeatures2 reqPhysicalFeatures = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    vkGetPhysicalDeviceFeatures2(a_physicalDevice, &reqPhysicalFeatures);

    // Tmp
    mExpect(bindless_supported);

    reqPhysicalFeatures.pNext      = &timelineSemaphoreFeature;
    timelineSemaphoreFeature.pNext = &dynamicRenderingFeature;
    if (bindless_supported)
    {
        dynamicRenderingFeature.pNext = &dstIndexingFeatures;
    }
    // End of bindless feature request

    VkDeviceCreateInfo createInfo{};
    createInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos    = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures     = nullptr;
    createInfo.enabledExtensionCount =
        static_cast<mU32>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.pNext                   = &reqPhysicalFeatures;
    if (vkCreateDevice(a_physicalDevice, &createInfo, nullptr,
                       &a_logicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device !");
    }

    vkGetDeviceQueue(a_logicalDevice, a_queueFamilyIndex, 0, &a_queue);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void create_vmaAllocator(VkInstance       a_instance,
                         VkPhysicalDevice a_physicalDevice, VkDevice a_device,
                         VmaAllocator& a_vmaAllocator)
{
    VmaAllocatorCreateInfo createInfo_vmaAllocator = {};
    createInfo_vmaAllocator.instance               = a_instance;
    createInfo_vmaAllocator.physicalDevice         = a_physicalDevice;
    createInfo_vmaAllocator.device                 = a_device;
    createInfo_vmaAllocator.vulkanApiVersion       = c_vulkanVersion;

    check_vkResult(
        vmaCreateAllocator(&createInfo_vmaAllocator, &a_vmaAllocator));
}

}  // namespace vulkan
}  // namespace m