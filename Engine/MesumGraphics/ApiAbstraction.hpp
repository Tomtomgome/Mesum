#pragma once

#include "MesumGraphics/Common.hpp"

#ifdef mIfDx12Enabled
#undef mIfDx12Enabled
#endif

#ifdef mIfVulkanEnabled
#undef mIfVulkanEnabled
#endif

#if (defined M_DX12_RENDERER) && (!defined M_VK_IMPLEMENTAITON)
#include "DX12Renderer/Includes.hpp"
#define mIfDx12Enabled(a_something) a_something
#else
#define mIfDx12Enabled(a_something)
#endif  // M_DX12_RENDERER

#if (defined M_VULKAN_RENDERER) && (!defined M_DX12_IMPLEMENTAITON)
#include "VulkanRenderer/Includes.hpp"
#define mIfVulkanEnabled(a_something) a_something
#else
#define mIfVulkanEnabled(a_something)
#endif  // M_VULKAN_RENDERER

///////////////////////////////////////////////////////////////////////////////
/// \addtogroup Graphics
/// \{
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// \brief Namespace grouping rendering api abstraction structures
///////////////////////////////////////////////////////////////////////////////
namespace m::aa
{
///////////////////////////////////////////////////////////////////////////////
/// \brief Define in place a function pointer
///
/// \param ret The return type of the function
/// \param name The name of the function
/// \vparam ... The variadic parameters of the function
///////////////////////////////////////////////////////////////////////////////
#define mDeclare_virtualFunction(ret, name, ...) \
    typedef ret (*name##Type)(__VA_ARGS__);      \
    name##Type name;

///////////////////////////////////////////////////////////////////////////////
/// \brief Used by mDeclare_virtualMemberFunction to swap parameters
///////////////////////////////////////////////////////////////////////////////
#define mDeclare_virtualMemberFunction__(ret, name, parentType, ...)    \
    mDeclare_virtualFunction(ret, name##Internal, parentType&,          \
                             __VA_ARGS__) template <typename... t_Args> \
    inline ret name(t_Args... a_args)                                   \
    {                                                                   \
        mAssert(name##Internal != nullptr);                             \
        return name##Internal(*this, a_args...);                        \
    }

///////////////////////////////////////////////////////////////////////////////
/// \brief Define in place a member function that has some virtual properties
/// thanks to an indirection with function pointer
///
/// \param parentType The type of the parent class/struct
/// \param ret The return type of the function
/// \param name The name of the function
/// \vparam ... The variadic parameters of the function
///////////////////////////////////////////////////////////////////////////////
#define mDeclare_virtualMemberFunction(parentType, ret, name, ...) \
    mDeclare_virtualMemberFunction__(ret, name, parentType, __VA_ARGS__)

///////////////////////////////////////////////////////////////////////////////
/// \brief Used to link the member function of an object to its implementation
///////////////////////////////////////////////////////////////////////////////
#define mLink_virtualMemberFunction(a_o, a_if) a_o.a_if##Internal = a_if

///////////////////////////////////////////////////////////////////////////////
/// \brief Used to link the member function of an object to its implementation
///////////////////////////////////////////////////////////////////////////////
#define mLink_virtualMemberFunctionEXT(a_o, a_if, a_f) a_o.a_if##Internal = a_f

struct mDevice
{
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Structure abstracting an adapter to a physical device
///////////////////////////////////////////////////////////////////////////////
struct mAdapter
{
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Initialization data of an adapter
    ///////////////////////////////////////////////////////////////////////////
    enum DeviceType
    {
        deviceOther,
        deviceIntegrated,
        deviceDescrete,
        deviceVirtual,
        deviceCpu
    };

    struct Limits
    {
    };

    struct Properties
    {
        mU32       idVendor;
        mU32       idDevice;
        DeviceType type;
        Limits     limits;
    };

    mDeclare_virtualMemberFunction(mAdapter, void, init);
    mDeclare_virtualMemberFunction(mAdapter, void, destroy);

    Properties properties;
    union
    {
        mIfDx12Enabled(struct { IDXGIAdapter4* adapter; } dx12;);
        mIfVulkanEnabled(struct {VkPhysicalDevice physicalDevice;} vulkan;);
    } internal;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Structure used to instantiate all other structures
///////////////////////////////////////////////////////////////////////////////
struct mApi
{
    ///////////////////////////////////////////////////////////////////////////
    /// \brief Initialization data of an api
    ///////////////////////////////////////////////////////////////////////////
    struct InitData
    {
        bool enableDebug;
        // Debug flags
    };

    mDeclare_virtualMemberFunction(mApi, void, init, mApi::InitData const&);
    mDeclare_virtualMemberFunction(mApi, void, destroy);
    mDeclare_virtualMemberFunction(mApi, void, enumerate_adapter,
                                   std::vector<mAdapter>& a_adapters);
    mDeclare_virtualFunction(mAdapter, create_adapter);

    bool debugEnabled;
    union
    {
        mIfDx12Enabled(struct { IDXGIFactory4* factory; } dx12;);
        mIfVulkanEnabled(struct
                         {
                             VkInstance               instance;
                             VkDebugUtilsMessengerEXT debugUtil;
                         } vulkan;);
    } internal;
};

mIfDx12Enabled(namespace dx12 { mApi create_api(); });
mIfVulkanEnabled(namespace vulkan { mApi create_api(); });

};  // namespace m::aa

///////////////////////////////////////////////////////////////////////////////
/// \}
///////////////////////////////////////////////////////////////////////////////