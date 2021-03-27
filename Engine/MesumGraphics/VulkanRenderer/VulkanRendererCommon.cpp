#include <VulkanRendererCommon.hpp>
#include <vector>

namespace m
{
namespace vulkan
{
extern const logging::ChannelID VK_RENDERER_ID = mLOG_GET_ID();

#ifdef M_DEBUG
const Bool g_enableValidationLayers = false;
#else
const Bool g_enableValidationLayers = true;
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

    return extensions;
}

void create_instance(VkInstance& a_InstaceToCreate) {
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

    auto                 extensions  = get_requiedExtensions();
    VkInstanceCreateInfo createInfo  = {};
    createInfo.sType                 = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo      = &appInfo;
    createInfo.enabledExtensionCount = static_cast<U32>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount       = 0;
    if (g_enableValidationLayers)
    {
        createInfo.enabledLayerCount =
            static_cast<U32>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
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
    mLOG_ERR_TO(VK_RENDERER_ID, "Validation layer : ", pCallbackData->pMessage);

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

void setup_debugMessenger(VkInstance const&         a_instance,
                          VkDebugUtilsMessengerEXT& a_debugUtil)
{
    if (!g_enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = callback_logDebugMessage;
    createInfo.pUserData       = nullptr;

    if (create_debugUtilsMessengerEXT(a_instance, &createInfo, nullptr,
                                      &a_debugUtil) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger");
    }
}

}  // namespace vulkan
}  // namespace m