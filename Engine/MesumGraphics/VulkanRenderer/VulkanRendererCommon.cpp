#include <VulkanRendererCommon.hpp>
#include <optional>
#include <set>
#include <vector>

namespace m
{
namespace vulkan
{
extern const logging::ChannelID VK_RENDERER_ID = mLOG_GET_ID();

#ifdef M_DEBUG
const Bool g_enableValidationLayers = true;
#else
const Bool g_enableValidationLayers = false;
#endif

const std::vector<const ShortChar*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const ShortChar*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

Bool check_validationLayerSupport()
{
    U32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const ShortChar* layerName : validationLayers)
    {
        Bool layerFound = false;

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

std::vector<const ShortChar*> get_requiedExtensions()
{
    std::vector<const ShortChar*> extensions;

    if (g_enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined M_WIN32
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined M_UNIX
#endif
    return extensions;
}

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
    appInfo.apiVersion         = VK_API_VERSION_1_2;

    auto                 extensions    = get_requiedExtensions();
    VkInstanceCreateInfo createInfo    = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledExtensionCount   = static_cast<U32>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount       = 0;
    if (g_enableValidationLayers)
    {
        createInfo.enabledLayerCount =
            static_cast<U32>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        //         VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        //         setup_debugUtilsMessengerCreateInfoExt(debugCreateInfo);
        //         createInfo.pNext =
        //             (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    U32 extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

#ifdef PRINT_EXTENTIONS
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                           extensions.data());

    mLOG_TO(VK_RENDERER_ID, "Available extensions");
    for (const auto& extension : extensions)
    {
        mLOG_TO(VK_RENDERER_ID, extension.extensionName);
    }
#endif  // PRINT_EXTENTIONS

    if (vkCreateInstance(&createInfo, nullptr, &a_InstaceToCreate))
    {
        throw std::runtime_error("failed to create instance !");
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL callback_logDebugMessage(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    switch (messageSeverity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            mLOG_TO(VK_RENDERER_ID,
                    "Validation layer : ", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            mLOG_WARN_TO(VK_RENDERER_ID,
                         "Validation layer : ", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            mLOG_ERR_TO(VK_RENDERER_ID,
                        "Validation layer : ", pCallbackData->pMessage);
            break;
        default: mInterrupt;
    }

    return VK_FALSE;
}

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

Bool check_deviceExtensionSupport(VkPhysicalDevice a_device)
{
    U32 extensionCount;
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

Bool check_deviceSuitable(VkPhysicalDevice device)
{
    Bool extensionsSupported = check_deviceExtensionSupport(device);
    // [TODO] Will do queue families support check later
    // [TODO] Will do swap chain compabilities check later
    return extensionsSupported;
}

void select_physicalDevice(VkInstance        a_instance,
                           VkPhysicalDevice& a_physicalDevice)
{
    U32 deviceCount = 0;
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

Bool find_graphicQueueFamilyIndex(VkPhysicalDevice a_physicalDevice,
                                  U32&             a_queueFamilyIndex)
{
    U32 queueFamilyCount = 0;
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

void create_logicalDevice(VkPhysicalDevice a_physicalDevice,
                          VkDevice& a_logicalDevice, VkQueue& a_queue)
{
    U32 queueFamilyIndex;
    find_graphicQueueFamilyIndex(a_physicalDevice, queueFamilyIndex);

    float                   queuePriority   = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos    = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures     = &deviceFeatures;
    createInfo.enabledExtensionCount =
        static_cast<U32>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(a_physicalDevice, &createInfo, nullptr,
                       &a_logicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device !");
    }

    vkGetDeviceQueue(a_logicalDevice, queueFamilyIndex, 0, &a_queue);
}

}  // namespace vulkan
}  // namespace m