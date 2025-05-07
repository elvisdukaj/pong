module;

#include <volk.h>
#include <vulkan/vulkan_beta.h>

#if defined(VK_USE_PLATFORM_METAL_EXT)
#include <vulkan/vulkan_metal.h>
#endif

export module vis.graphic.vulkan.vkh:constants;

export namespace vkh {
// Extensions names
constexpr const char* KHRGetPhysicalDeviceProperties2ExtensionName =
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
constexpr const char* KHRGetSurfaceCapabilities2ExtensionName = VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME;
#if defined(VK_USE_PLATFORM_METAL_EXT)
constexpr const char* EXTMetalSurfaceExtensionName = VK_EXT_METAL_SURFACE_EXTENSION_NAME;
#endif
constexpr const char* KHRSwapchainExtensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
constexpr const char* KHRPortabilityEnumerationExtensionName = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
constexpr const char* KHRFormatFeatureFlags2ExtensionName = VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME;
constexpr const char* KHRPortabilitySubsetExtensionName = VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME;

} // namespace vkh