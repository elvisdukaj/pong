module;

#include <cassert>
#include <volk.h>

#include "vk_enum_string_helper.h"

#include <vulkan/vulkan_beta.h>
#include <vulkan/vulkan_metal.h>

export module vis.graphic.vulkan.vkh;

export import std;
import vis.window;

namespace helper {

template <std::integral T> constexpr uint32_t api_version_major(T const version) {
  return (static_cast<uint32_t>(version) >> 22U) & 0x7FU;
}

template <std::integral T> constexpr uint32_t api_version_minor(T const version) {
  return (static_cast<uint32_t>(version) >> 12U) & 0x3FFU;
}

template <std::integral T> constexpr uint32_t api_version_patch(T const version) {
  return static_cast<uint32_t>(version) & 0xFFFU;
}

template <std::integral T> constexpr uint32_t api_version_variant(T const version) {
  return static_cast<uint32_t>(version) >> 29U;
}

template <std::integral T>
constexpr uint32_t make_api_version(T const variant, T const major, T const minor, T const patch) {
  return ((((uint32_t)(variant)) << 29U) | (((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) |
          ((uint32_t)(patch)));
}

template <std::integral T> constexpr uint32_t make_version(T const major, T const minor, T const patch) {
  return ((((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch)));
}

std::string vk_version_to_string(uint32_t version) noexcept {
  return std::format("{}.{}.{}", api_version_major(version), api_version_minor(version), api_version_patch(version));
}

std::string vk_api_version_to_string(uint32_t version) noexcept {
  return std::format("{}.{}.{}-{}", api_version_major(version), api_version_minor(version), api_version_patch(version),
                     api_version_variant(version));
}

} // namespace helper

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

enum class Result {
  Success = VK_SUCCESS,
  NotReady = VK_NOT_READY,
  Timeout = VK_TIMEOUT,
  EventSet = VK_EVENT_SET,
  EventReset = VK_EVENT_RESET,
  Incomplete = VK_INCOMPLETE,
  ErrorOutOfHostMemory = VK_ERROR_OUT_OF_HOST_MEMORY,
  ErrorOutOfDeviceMemory = VK_ERROR_OUT_OF_DEVICE_MEMORY,
  ErrorInitializationFailed = VK_ERROR_INITIALIZATION_FAILED,
  ErrorDeviceLost = VK_ERROR_DEVICE_LOST,
  ErrorMemoryMapFailed = VK_ERROR_MEMORY_MAP_FAILED,
  ErrorLayerNotPresent = VK_ERROR_LAYER_NOT_PRESENT,
  ErrorExtensionNotPresent = VK_ERROR_EXTENSION_NOT_PRESENT,
  ErrorFeatureNotPresent = VK_ERROR_FEATURE_NOT_PRESENT,
  ErrorIncompatibleDriver = VK_ERROR_INCOMPATIBLE_DRIVER,
  ErrorTooManyObjects = VK_ERROR_TOO_MANY_OBJECTS,
  ErrorFormatNotSupported = VK_ERROR_FORMAT_NOT_SUPPORTED,
  ErrorFragmentedPool = VK_ERROR_FRAGMENTED_POOL,
  ErrorUnknown = VK_ERROR_UNKNOWN,
  ErrorOutOfPoolMemory = VK_ERROR_OUT_OF_POOL_MEMORY,
  ErrorOutOfPoolMemoryKHR = VK_ERROR_OUT_OF_POOL_MEMORY_KHR,
  ErrorInvalidExternalHandle = VK_ERROR_INVALID_EXTERNAL_HANDLE,
  ErrorInvalidExternalHandleKHR = VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR,
  ErrorFragmentation = VK_ERROR_FRAGMENTATION,
  ErrorFragmentationEXT = VK_ERROR_FRAGMENTATION_EXT,
  ErrorInvalidOpaqueCaptureAddress = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS,
  ErrorInvalidDeviceAddressEXT = VK_ERROR_INVALID_DEVICE_ADDRESS_EXT,
  ErrorInvalidOpaqueCaptureAddressKHR = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR,
  PipelineCompileRequired = VK_PIPELINE_COMPILE_REQUIRED,
  ErrorPipelineCompileRequiredEXT = VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT,
  PipelineCompileRequiredEXT = VK_PIPELINE_COMPILE_REQUIRED_EXT,
  ErrorSurfaceLostKHR = VK_ERROR_SURFACE_LOST_KHR,
  ErrorNativeWindowInUseKHR = VK_ERROR_NATIVE_WINDOW_IN_USE_KHR,
  SuboptimalKHR = VK_SUBOPTIMAL_KHR,
  ErrorOutOfDateKHR = VK_ERROR_OUT_OF_DATE_KHR,
  ErrorIncompatibleDisplayKHR = VK_ERROR_INCOMPATIBLE_DISPLAY_KHR,
  ErrorValidationFailedEXT = VK_ERROR_VALIDATION_FAILED_EXT,
  ErrorInvalidShaderNV = VK_ERROR_INVALID_SHADER_NV,
  ErrorImageUsageNotSupportedKHR = VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR,
  ErrorVideoPictureLayoutNotSupportedKHR = VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR,
  ErrorVideoProfileOperationNotSupportedKHR = VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR,
  ErrorVideoProfileFormatNotSupportedKHR = VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR,
  ErrorVideoProfileCodecNotSupportedKHR = VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR,
  ErrorVideoStdVersionNotSupportedKHR = VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR,
  ErrorInvalidDrmFormatModifierPlaneLayoutEXT = VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT,
  ErrorNotPermittedKHR = VK_ERROR_NOT_PERMITTED_KHR,
  ErrorNotPermittedEXT = VK_ERROR_NOT_PERMITTED_EXT,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  ErrorFullScreenExclusiveModeLostEXT = VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT,
#endif /*VK_USE_PLATFORM_WIN32_KHR*/
  ThreadIdleKHR = VK_THREAD_IDLE_KHR,
  ThreadDoneKHR = VK_THREAD_DONE_KHR,
  OperationDeferredKHR = VK_OPERATION_DEFERRED_KHR,
  OperationNotDeferredKHR = VK_OPERATION_NOT_DEFERRED_KHR,
  ErrorInvalidVideoStdParametersKHR = VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR,
  ErrorCompressionExhaustedEXT = VK_ERROR_COMPRESSION_EXHAUSTED_EXT,
  IncompatibleShaderBinaryEXT = VK_INCOMPATIBLE_SHADER_BINARY_EXT,
  ErrorIncompatibleShaderBinaryEXT = VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT,
  PipelineBinaryMissingKHR = VK_PIPELINE_BINARY_MISSING_KHR,
  ErrorNotEnoughSpaceKHR = VK_ERROR_NOT_ENOUGH_SPACE_KHR
};

template <typename T, typename Func, typename... Args> std::vector<T> enumerate(Func func, Args... args) {
  std::vector<T> container;
  uint32_t count{};

  // TODO: some function could return incomplete and need to be polled in a loop
  func(args..., &count, nullptr);
  container.resize(count);
  func(args..., &count, container.data());

  return container;
}

template <typename FlagBitsType> struct FlagTraits {
  static constexpr bool is_bit_mask = false;
};

template <typename BitType> class Flags {
public:
  using MaskType = typename std::underlying_type<BitType>::type;

  // constructors
  constexpr Flags() noexcept : mask(0) {}

  constexpr Flags(BitType bit) noexcept : mask(static_cast<MaskType>(bit)) {}
  constexpr Flags(Flags<BitType> const& rhs) noexcept = default;

  constexpr explicit Flags(MaskType flags) noexcept : mask(flags) {}

  constexpr Flags<BitType> operator&(Flags<BitType> const& rhs) const noexcept {
    return Flags<BitType>(mask & rhs.mask);
  }

  constexpr Flags<BitType> operator|(Flags<BitType> const& rhs) const noexcept {
    return Flags<BitType>(mask | rhs.mask);
  }

  constexpr Flags<BitType> operator^(Flags<BitType> const& rhs) const noexcept {
    return Flags<BitType>(mask ^ rhs.mask);
  }

  constexpr Flags<BitType> operator~() const noexcept {
    return Flags<BitType>(mask ^ FlagTraits<BitType>::allFlags.m_mask);
  }

  // assignment operators
  constexpr Flags<BitType>& operator=(Flags<BitType> const& rhs) noexcept = default;

  constexpr Flags<BitType>& operator|=(Flags<BitType> const& rhs) noexcept {
    mask |= rhs.mask;
    return *this;
  }

  constexpr Flags<BitType>& operator&=(Flags<BitType> const& rhs) noexcept {
    mask &= rhs.mask;
    return *this;
  }

  constexpr Flags<BitType>& operator^=(Flags<BitType> const& rhs) noexcept {
    mask ^= rhs.mask;
    return *this;
  }

  // cast operators
  explicit constexpr operator bool() const noexcept {
    return !!mask;
  }

  explicit constexpr operator MaskType() const noexcept {
    return mask;
  }

private:
  MaskType mask;
};

enum class InstanceCreateFlagBits : VkInstanceCreateFlags {
  EnumeratePortabilityKHR = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
};

using InstanceCreateFlags = Flags<InstanceCreateFlagBits>;

// forward and using
class Context;
class InstanceBuilder;
class Instance;
class Surface;
class PhysicalDevice;
class PhysicalDeviceSelector;
class CommandPool;
class Device;
class RenderPass;
class RenderPassBuilder;
class Semaphore;
class Fence;

class ApplicationInfoBuilder {
public:
  ApplicationInfoBuilder& with_next(const void* required_next) noexcept {
    next = required_next;
    return *this;
  }

  ApplicationInfoBuilder& with_application_name(std::string_view requestd_application_name) noexcept {
    application_name = requestd_application_name;
    return *this;
  }

  ApplicationInfoBuilder& with_application_version(uint32_t requested_application_version) noexcept {
    application_version = requested_application_version;
    return *this;
  }

  ApplicationInfoBuilder& with_application_version(uint32_t variant, uint32_t maj, uint32_t min,
                                                   uint32_t patch) noexcept {
    application_version = helper::make_api_version(variant, maj, min, patch);
    return *this;
  }

  ApplicationInfoBuilder& with_application_version(uint32_t maj, uint32_t min, uint32_t patch) noexcept {
    application_version = helper::make_version(maj, min, patch);
    return *this;
  }

  ApplicationInfoBuilder& with_engine_name(std::string_view requested_engine_name) noexcept {
    engine_name = requested_engine_name;
    return *this;
  }

  ApplicationInfoBuilder& with_engine_version(uint32_t requested_engine_version) noexcept {
    engine_version = requested_engine_version;
    return *this;
  }

  ApplicationInfoBuilder& with_engine_version(uint32_t variant, uint32_t maj, uint32_t min, uint32_t patch) noexcept {
    application_version = helper::make_api_version(variant, maj, min, patch);
    return *this;
  }

  ApplicationInfoBuilder& with_engine_version(uint32_t maj, uint32_t min, uint32_t patch) noexcept {
    application_version = helper::make_version(maj, min, patch);
    return *this;
  }

  ApplicationInfoBuilder& with_api_version(uint32_t requested_api_version) noexcept {
    api_version = requested_api_version;
    return *this;
  }

  ApplicationInfoBuilder& with_api_version(uint32_t variant, uint32_t maj, uint32_t min, uint32_t patch) noexcept {
    application_version = helper::make_api_version(variant, maj, min, patch);
    return *this;
  }

  ApplicationInfoBuilder& with_api_version(uint32_t maj, uint32_t min, uint32_t patch) noexcept {
    application_version = helper::make_version(maj, min, patch);
    return *this;
  }

  VkApplicationInfo create() const noexcept {
    return VkApplicationInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = next,
        .pApplicationName = application_name.c_str(),
        .applicationVersion = application_version,
        .pEngineName = engine_name.c_str(),
        .engineVersion = engine_version,
        .apiVersion = api_version,
    };
  }

private:
  const void* next = nullptr;
  std::string application_name = "";
  uint32_t application_version = 0;
  std::string engine_name = "";
  uint32_t engine_version = 0;
  uint32_t api_version = 0;
};

struct InstanceCreateInfoBuilder {
  VkInstanceCreateInfo create() {
    return VkInstanceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = next,
        .flags = static_cast<VkInstanceCreateFlags>(flags),
        .pApplicationInfo = application_info,
        .enabledLayerCount = static_cast<uint32_t>(enabled_layer_names.size()),
        .ppEnabledLayerNames = enabled_layer_names.data(),
        .enabledExtensionCount = static_cast<uint32_t>(enabled_extension_names.size()),
        .ppEnabledExtensionNames = enabled_extension_names.data(),
    };
  }

  InstanceCreateInfoBuilder& with_next(const void* requested_next) noexcept {
    next = requested_next;
    return *this;
  }

  InstanceCreateInfoBuilder& add_flags(InstanceCreateFlags requested_flags) noexcept {
    flags |= requested_flags;
    return *this;
  }

  InstanceCreateInfoBuilder& with_application_info(const VkApplicationInfo* requested_application_info) noexcept {
    application_info = requested_application_info;
    return *this;
  }

  InstanceCreateInfoBuilder& add_required_layer(const char* required_layer_name) noexcept {
    enabled_layer_names.push_back(required_layer_name);
    return *this;
  }

  InstanceCreateInfoBuilder& add_required_layers(std::span<const char*> required_layers) noexcept {
    enabled_layer_names.insert(enabled_layer_names.end(), begin(required_layers), end(required_layers));
    return *this;
  }

  InstanceCreateInfoBuilder& add_required_extension(const char* required_extension_name) noexcept {
    enabled_extension_names.push_back(required_extension_name);
    return *this;
  }

  InstanceCreateInfoBuilder& add_required_extensions(std::span<const char*> required_extensions) noexcept {
    enabled_extension_names.insert(enabled_extension_names.end(), begin(required_extensions), end(required_extensions));
    return *this;
  }

  std::vector<const char*> get_enabled_layers() const noexcept {
    return enabled_layer_names;
  }

  std::vector<const char*> get_enabled_extensions() const noexcept {
    return enabled_extension_names;
  }

private:
  const void* next = nullptr;
  InstanceCreateFlags flags;
  const VkApplicationInfo* application_info;
  std::vector<const char*> enabled_layer_names;
  std::vector<const char*> enabled_extension_names;
};

class PhysicalDeviceFeatures2Builder {
public:
  using NativeType = VkPhysicalDeviceFeatures2;

  PhysicalDeviceFeatures2Builder() noexcept {
    native = NativeType{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = nullptr,
        .features = {},
    };
  }

  PhysicalDeviceFeatures2Builder& with_next(VkBaseInStructure* next = nullptr) noexcept {
    native.pNext = next;
    return *this;
  }

  NativeType build() const noexcept {
    return native;
  }

private:
  NativeType native;
};

class PhysicalDeviceProperties2Builder {
public:
  using NativeType = VkPhysicalDeviceProperties2;

  PhysicalDeviceProperties2Builder() noexcept {
    native = NativeType{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = nullptr,
        .properties = {},
    };
  }

  PhysicalDeviceProperties2Builder& with_next(VkBaseInStructure* next = nullptr) noexcept {
    native.pNext = next;
    return *this;
  }

  NativeType build() const noexcept {
    return native;
  }

private:
  NativeType native;
};

class Context {
public:
  Context() {
    api_version = volkGetInstanceVersion();
    available_layers = enumerate<VkLayerProperties>(vkEnumerateInstanceLayerProperties);
    available_extensions = enumerate<VkExtensionProperties>(vkEnumerateInstanceExtensionProperties, nullptr);
    // TODO: serialize
    std::println("context:");
    std::println("  version: {}", helper::vk_version_to_string(api_version));
    std::println("  available layers:");
    for (const auto& layer : available_layers) {
      std::println("    - {}", std::string_view{layer.layerName});
    }
    std::println("  available extensions:");
    for (const auto& ext : available_extensions) {
      std::println("    - {}", std::string_view{ext.extensionName});
    }
  }

  [[nodiscard]] VkInstance create_instance(VkInstanceCreateInfo& create_info, const VkAllocationCallbacks* allocator) {
    VkInstance instance = VK_NULL_HANDLE;
    auto res = vkCreateInstance(&create_info, allocator, &instance);

    if (res != VK_SUCCESS) {
      throw std::runtime_error{"Unable to create a Vulkan Instance"};
    }

    std::call_once(volk_instance_flag, [instance]() { volkLoadInstance(instance); });

    return instance;
  }

  uint32_t get_api_version() const noexcept {
    return api_version;
  }

  [[nodiscard]] bool has_extension(std::string_view extension_name) const noexcept {
    return std::ranges::find_if(available_extensions, [extension_name](const auto& extension) -> bool {
             return std::string_view{extension.extensionName} == extension_name;
           }) != end(available_extensions);
  }

  [[nodiscard]] bool has_extensions(std::span<const char*> extensions) const noexcept {
    return std::ranges::all_of(extensions,
                               [this](const char* extension_name) { return has_extension(extension_name); });
  }

  [[nodiscard]] bool has_layer(std::string_view layer_name) const {
    return std::ranges::find_if(available_layers, [layer_name](const auto& layer) -> bool {
             return std::string_view{layer.layerName} == layer_name;
           }) != end(available_layers);
  }

  [[nodiscard]] bool has_layers(std::span<const char*> layers) const noexcept {
    return std::ranges::all_of(layers, [this](const char* layer_name) { return has_layer(layer_name); });
  }

private:
  std::once_flag volk_instance_flag;
  uint32_t api_version = 0;
  std::vector<VkExtensionProperties> available_extensions;
  std::vector<VkLayerProperties> available_layers;
};

class Instance {
public:
  using NativeType = VkInstance;

  explicit Instance(std::nullptr_t)
      : context{nullptr}, instance{VK_NULL_HANDLE}, api_version{}, layers{}, extensions{} {}

  Instance(Context* context, VkInstance instance, uint32_t required_version, std::vector<const char*> required_layers,
           std::vector<const char*> required_extensions)
      : context{context}, instance{instance}, api_version{required_version} {
    layers.insert(layers.end(), begin(required_layers), end(required_layers));
    extensions.insert(extensions.end(), begin(required_extensions), end(required_extensions));
    // TODO: serialize
    // config["version"] = helper::vk_version_to_string(api_version);
    // TODO: serialize
    std::println("instance:");
    std::println("  version: {}", helper::vk_version_to_string(api_version));
    std::println("  layers:");
    for (const auto& layer : layers)
      std::println("  - {}", layer);
    std::println("  extensions:");
    for (const auto& extension : extensions)
      std::println("  - {}", extension);
  }

  ~Instance() {
    if (instance != VK_NULL_HANDLE) {
      vkDestroyInstance(instance, nullptr);
    }
  }

  // TODO: serialize
  // YAML::Node dump() const {
  // return YAML::Clone(config);
  // }

  Instance(const Instance& other) = delete;
  Instance& operator=(const Instance& other) = delete;

  Instance(Instance&& other) noexcept
      : context{other.context},
        instance{other.instance},
        api_version{other.api_version},
        layers{std::move(other.layers)},
        extensions{std::move(other.extensions)} {
    other.context = nullptr;
    other.instance = VK_NULL_HANDLE;
    other.api_version = 0;
  }

  Instance& operator=(Instance&& other) noexcept {
    std::swap(context, other.context);
    std::swap(instance, other.instance);
    std::swap(api_version, other.api_version);
    std::swap(layers, other.layers);
    std::swap(extensions, other.extensions);
    return *this;
  }

  [[nodiscard]] uint32_t get_api_version() const noexcept {
    return api_version;
  }

  NativeType native_handle() const noexcept {
    return instance;
  }

private:
  Context* context = nullptr;
  NativeType instance = VK_NULL_HANDLE;
  uint32_t api_version = 0;
  std::vector<const char*> layers;
  std::vector<const char*> extensions;
  // TODO: serialize
  // YAML::Node config;
};

class InstanceBuilder {
public:
  explicit InstanceBuilder(Context& context) : context{context} {}

  InstanceBuilder& with_minimum_required_instance_version(int variant, int major, int minor, int patch) noexcept {
    minimum_instance_version = helper::make_api_version(variant, major, minor, patch);
    return *this;
  }

  InstanceBuilder& with_maximum_required_instance_version(int variant, int major, int minor, int patch) noexcept {
    maximum_instance_version = helper::make_api_version(variant, major, minor, patch);
    return *this;
  }

  InstanceBuilder& with_app_name(std::string_view name) noexcept {
    app_info_builder.with_application_name(name.data());
    return *this;
  }

  InstanceBuilder& with_app_version(uint32_t version) noexcept {
    app_info_builder.with_application_version(version);
    return *this;
  }

  InstanceBuilder& with_app_version(uint32_t major, uint32_t minor, uint32_t patch) noexcept {
    app_info_builder.with_application_version(major, minor, patch);
    return *this;
  }

  InstanceBuilder& with_engine_name(std::string_view name) noexcept {
    app_info_builder.with_engine_name(name);
    return *this;
  }

  InstanceBuilder& with_engine_version(uint32_t version) noexcept {
    app_info_builder.with_engine_version(version);
    return *this;
  }

  InstanceBuilder& with_engine_version(uint32_t major, uint32_t minor, uint32_t patch) noexcept {
    app_info_builder.with_engine_version(major, minor, patch);
    return *this;
  }

  InstanceBuilder& add_required_layer(const char* layer_name) noexcept {
    instance_create_info_builder.add_required_layer(layer_name);
    return *this;
  }

  InstanceBuilder& add_required_layers(std::span<const char*> layers) noexcept {
    instance_create_info_builder.add_required_layers(layers);
    return *this;
  }

  InstanceBuilder& add_required_extension(const char* extension) noexcept {
    instance_create_info_builder.add_required_extension(extension);
    return *this;
  }

  InstanceBuilder& add_required_extensions(std::span<const char*> extensions) noexcept {
    instance_create_info_builder.add_required_extensions(extensions);
    return *this;
  }

  InstanceBuilder& with_app_flags(InstanceCreateFlags instance_flag) noexcept {
    instance_create_info_builder.add_flags(instance_flag);
    return *this;
  }

  Instance build() {
    maximum_instance_version = std::min(maximum_instance_version, context.get_api_version());
    if (minimum_instance_version > maximum_instance_version) {
      throw std::runtime_error{std::format("The minimum vulkan version is {} but the minimum required is {}",
                                           helper::vk_version_to_string(context.get_api_version()),
                                           helper::vk_version_to_string(maximum_instance_version))};
    }

    auto required_api_version = std::max(minimum_instance_version, maximum_instance_version);
    app_info_builder.with_api_version(required_api_version);

    auto app_info = app_info_builder.create();
    auto create_info = instance_create_info_builder.with_application_info(&app_info).create();
    auto native_instance = context.create_instance(create_info, nullptr);

    return {&context, native_instance, required_api_version, instance_create_info_builder.get_enabled_layers(),
            instance_create_info_builder.get_enabled_extensions()};
  }

private:
  Context& context;

  uint32_t minimum_instance_version = helper::make_api_version(0, 1, 0, 0);
  uint32_t maximum_instance_version = helper::make_api_version(0, 1, 4, 0);

  ApplicationInfoBuilder app_info_builder;
  InstanceCreateInfoBuilder instance_create_info_builder;
};

class Surface {
  friend class SurfaceBuilder;

public:
  using NativeType = VkSurfaceKHR;

  explicit Surface(std::nullptr_t) : instance{nullptr}, handle{VK_NULL_HANDLE} {}

  Surface(Surface&) = delete;
  Surface& operator=(Surface&) = delete;

  Surface(Surface&& other) : instance{other.instance}, handle{other.handle} {
    other.instance = nullptr;
    other.handle = VK_NULL_HANDLE;
  }

  Surface& operator=(Surface&& other) {
    std::swap(instance, other.instance);
    std::swap(handle, other.handle);
    return *this;
  }

  ~Surface() {
    if (handle != VK_NULL_HANDLE) {
      vkDestroySurfaceKHR(instance->native_handle(), handle, nullptr);
    }
  }

  NativeType native_handle() const noexcept {
    return handle;
  }

private:
  Surface(Instance* instance, VkSurfaceKHR handle) : instance{instance}, handle{handle} {}

private:
  Instance* instance = nullptr;
  VkSurfaceKHR handle = VK_NULL_HANDLE;
};

class SurfaceBuilder {
public:
  explicit SurfaceBuilder(Instance* instance, ::vis::Window* window) : instance{instance}, window{window} {}

  [[nodiscard]] Surface build() const noexcept {
    auto vk_surface = window->create_renderer_surface(instance->native_handle(), nullptr);
    return {instance, vk_surface};
  }

private:
  Instance* instance;
  vis::Window* window;
};

class Device {
  friend class PhysicalDevice;

public:
  using NativeType = VkDevice;
  explicit Device(std::nullptr_t) : handle{VK_NULL_HANDLE}, /*allocator{VK_NULL_HANDLE},*/ graphic_queue_index{} {}

  Device(const Device& other) = delete;
  Device& operator=(const Device& other) = delete;

  Device(Device&& other) noexcept
      : handle{other.handle}, /*allocator{other.allocator},*/ graphic_queue_index{other.graphic_queue_index} {
    other.handle = VK_NULL_HANDLE;
    // other.allocator = nullptr;
  }

  Device& operator=(Device&& other) noexcept {
    std::swap(handle, other.handle);
    // std::swap(allocator, other.allocator);
    std::swap(graphic_queue_index, other.graphic_queue_index);
    return *this;
  }

  ~Device() {
    if (handle != VK_NULL_HANDLE) {
      vkDestroyDevice(handle, nullptr);
      handle = VK_NULL_HANDLE;
    }
  }

private:
  Device(VkDevice device, /*VmaAllocator allocator,*/ std::size_t graphic_queue_index)
      : handle{device}, /*allocator{allocator},*/ graphic_queue_index{graphic_queue_index} {}

private:
  NativeType handle = VK_NULL_HANDLE;
  // VmaAllocator allocator = VK_NULL_HANDLE;
  std::size_t graphic_queue_index = 0;
};

class PhysicalDevice {
  friend class PhysicalDeviceSelector;

public:
  using NativeType = VkPhysicalDevice;
  PhysicalDevice(std::nullptr_t) noexcept : handle{nullptr}, surface{nullptr} {}

  VkPhysicalDeviceFeatures2 get_features2() const noexcept {
    (void)(surface);
    auto vk_features = PhysicalDeviceFeatures2Builder{}.build();
    vkGetPhysicalDeviceFeatures2(handle, &vk_features);
    return vk_features;
  }

  VkPhysicalDeviceProperties2 get_properties2() const noexcept {
    auto vk_props = PhysicalDeviceProperties2Builder{}.build();
    vkGetPhysicalDeviceProperties2(handle, &vk_props);
    return vk_props;
  }

  std::vector<VkLayerProperties> get_device_layer_properties() const noexcept {
    return enumerate<VkLayerProperties>(vkEnumerateDeviceLayerProperties, handle);
  }

  std::vector<VkLayerProperties> get_layers() const noexcept {
    return enumerate<VkLayerProperties>(vkEnumerateDeviceLayerProperties, handle);
  }

  std::vector<VkExtensionProperties> get_extensions(const char* layerName = nullptr) const noexcept {
    return enumerate<VkExtensionProperties>(vkEnumerateDeviceExtensionProperties, handle, layerName);
  }

  std::vector<VkQueueFamilyProperties2> get_queue_famylies() const noexcept {
    return enumerate<VkQueueFamilyProperties2>(vkGetPhysicalDeviceQueueFamilyProperties2, handle);
  }

  std::string device_name() const noexcept {
    return std::string{properties.properties.deviceName};
  }

  uint32_t device_api_version() const noexcept {
    return properties.properties.apiVersion;
  }

private:
  PhysicalDevice(NativeType device, Surface* surface) noexcept : handle{device}, surface{surface} {
    features = get_features2();
    properties = get_properties2();
    available_layers = get_layers();
    available_extensions = get_extensions();

    for (const auto& layer : available_layers) {
      auto extension_for_layer = get_extensions(layer.layerName);
      available_extensions.insert(end(available_extensions), begin(extension_for_layer), end(extension_for_layer));
    }

    available_queue_families = get_queue_famylies();
  }

  std::string serialize() const noexcept {
    // TODO: refactoring the serialization
    std::string result;

    result += std::format("  - name: {}\n"
                          "    version: {}\n"
                          "    type: {}\n",
                          std::string_view{properties.properties.deviceName},
                          helper::vk_api_version_to_string(properties.properties.apiVersion),
                          string_VkPhysicalDeviceType(properties.properties.deviceType));

    result += "    layers:\n";
    for (const auto& layer : available_layers)
      result += std::format("      - {}\n", std::string_view{layer.layerName});

    result += "    extensions:\n";
    for (const auto& extension : available_extensions)
      result += std::format("      - {}\n", std::string_view{extension.extensionName});

    result += std::format("    features:\n");

#define ENUMERATE_FEATURE(feature) result += std::format("      - {}: {}\n", #feature, bool(features.features.feature));

    ENUMERATE_FEATURE(robustBufferAccess);
    ENUMERATE_FEATURE(fullDrawIndexUint32);
    ENUMERATE_FEATURE(imageCubeArray);
    ENUMERATE_FEATURE(independentBlend);
    ENUMERATE_FEATURE(geometryShader);
    ENUMERATE_FEATURE(tessellationShader);
    ENUMERATE_FEATURE(sampleRateShading);
    ENUMERATE_FEATURE(dualSrcBlend);
    ENUMERATE_FEATURE(logicOp);
    ENUMERATE_FEATURE(multiDrawIndirect);
    ENUMERATE_FEATURE(drawIndirectFirstInstance);
    ENUMERATE_FEATURE(depthClamp);
    ENUMERATE_FEATURE(depthBiasClamp);
    ENUMERATE_FEATURE(fillModeNonSolid);
    ENUMERATE_FEATURE(depthBounds);
    ENUMERATE_FEATURE(wideLines);
    ENUMERATE_FEATURE(largePoints);
    ENUMERATE_FEATURE(alphaToOne);
    ENUMERATE_FEATURE(multiViewport);
    ENUMERATE_FEATURE(samplerAnisotropy);
    ENUMERATE_FEATURE(textureCompressionETC2);
    ENUMERATE_FEATURE(textureCompressionASTC_LDR);
    ENUMERATE_FEATURE(textureCompressionBC);
    ENUMERATE_FEATURE(occlusionQueryPrecise);
    ENUMERATE_FEATURE(pipelineStatisticsQuery);
    ENUMERATE_FEATURE(vertexPipelineStoresAndAtomics);
    ENUMERATE_FEATURE(fragmentStoresAndAtomics);
    ENUMERATE_FEATURE(shaderTessellationAndGeometryPointSize);
    ENUMERATE_FEATURE(shaderImageGatherExtended);
    ENUMERATE_FEATURE(shaderStorageImageExtendedFormats);
    ENUMERATE_FEATURE(shaderStorageImageMultisample);
    ENUMERATE_FEATURE(shaderStorageImageReadWithoutFormat);
    ENUMERATE_FEATURE(shaderStorageImageWriteWithoutFormat);
    ENUMERATE_FEATURE(shaderUniformBufferArrayDynamicIndexing);
    ENUMERATE_FEATURE(shaderSampledImageArrayDynamicIndexing);
    ENUMERATE_FEATURE(shaderStorageBufferArrayDynamicIndexing);
    ENUMERATE_FEATURE(shaderStorageImageArrayDynamicIndexing);
    ENUMERATE_FEATURE(shaderClipDistance);
    ENUMERATE_FEATURE(shaderCullDistance);
    ENUMERATE_FEATURE(shaderFloat64);
    ENUMERATE_FEATURE(shaderInt64);
    ENUMERATE_FEATURE(shaderInt16);
    ENUMERATE_FEATURE(shaderResourceResidency);

    result.pop_back();
    return result;
  }

private:
  NativeType handle = VK_NULL_HANDLE;
  Surface* surface = nullptr;
  VkPhysicalDeviceFeatures2 features;
  VkPhysicalDeviceProperties2 properties;
  std::vector<VkLayerProperties> available_layers;
  std::vector<VkExtensionProperties> available_extensions;
  std::vector<VkQueueFamilyProperties2> available_queue_families;
};

class PhysicalDeviceSelector {
public:
  explicit PhysicalDeviceSelector(Instance& instance, Surface* surface = nullptr)
      : instance{instance}, surface{surface} {}

  PhysicalDeviceSelector& add_required_extensions(std::span<const char*> extensions) {
    required_gpu_extensions.insert(std::end(required_gpu_extensions), std::begin(extensions), std::end(extensions));
    return *this;
  }

  PhysicalDeviceSelector& add_required_extensions(const char* extension) {
    required_gpu_extensions.push_back(extension);
    return *this;
  }

  PhysicalDeviceSelector& add_required_layer(const char* layer_name) noexcept {
    required_gpu_layers.push_back(layer_name);
    return *this;
  }

  PhysicalDeviceSelector& add_required_layers(std::span<const char*> layers) noexcept {
    required_gpu_layers.insert(required_gpu_layers.end(), begin(layers), end(layers));
    return *this;
  }

  PhysicalDeviceSelector& with_surface(Surface* desired_surface) {
    surface = desired_surface;
    return *this;
  }

  std::vector<PhysicalDevice> enumerate_all() const noexcept {
    // clang-format off
    auto devices = enumerate<VkPhysicalDevice>(vkEnumeratePhysicalDevices, instance.native_handle())
         | std::views::transform([this](VkPhysicalDevice vk_device) -> PhysicalDevice {
             return PhysicalDevice{vk_device, surface};
           })
        | std::ranges::to<std::vector<PhysicalDevice>>();
    // clang-format on

    // TODO: serialize
    std::println("physical devices:");
    for (const auto& d : devices)
      std::println("{}", d.serialize());

    return devices;
  }

private:
  Instance& instance;
  Surface* surface = nullptr;

  std::vector<const char*> required_gpu_extensions;
  std::vector<const char*> required_gpu_layers;
};

#if 0
class PhysicalDevice {
  friend class PhysicalDeviceSelector;

public:
  using NativeType = VkPhysicalDevice;
  PhysicalDevice() : physical_device{nullptr}, surface{nullptr} {}

  void swap(PhysicalDevice& other) noexcept {
    std::swap(physical_device, other.physical_device);
    std::swap(surface, other.surface);
    std::swap(available_properties, other.available_properties);
    std::swap(available_features, other.available_features);
    std::swap(available_layers, other.available_layers);
    std::swap(available_extensions, other.available_extensions);
    std::swap(available_queue_families, other.available_queue_families);
    std::swap(required_layers, other.required_layers);
    std::swap(required_extensions, other.required_extensions);
    std::swap(available_graphic_queue_indexes, other.available_graphic_queue_indexes);
    std::swap(available_transfer_queue_indexes, other.available_transfer_queue_indexes);
    std::swap(available_compute_queue_indexes, other.available_compute_queue_indexes);
    // std::swap(feature_chain, other.feature_chain);
    this->configuration = YAML::Clone(other.configuration);
  }

  PhysicalDevice(PhysicalDevice&) = delete;
  PhysicalDevice& operator=(PhysicalDevice&) = delete;

  PhysicalDevice(PhysicalDevice&& other) noexcept
      : physical_device{nullptr}, surface{nullptr}, feature_chain(std::move(other.feature_chain)) {
    swap(other);
  }

  PhysicalDevice& operator=(PhysicalDevice&& other) noexcept {
    swap(other);
    feature_chain = std::move(other.feature_chain);
    return *this;
  }

  const std::vector<uint32_t>& get_graphic_queue_indexes() const noexcept {
    return available_graphic_queue_indexes;
  }

  std::string_view name() const noexcept {
    return std::string_view{available_properties.properties.deviceName};
  }

  bool has_any(std::span<vk::PhysicalDeviceType> types) const noexcept {
    return std::ranges::any_of(
        types, [this](vk::PhysicalDeviceType type) { return available_properties.properties.deviceType == type; });
  }

  bool has_not_any(std::span<vk::PhysicalDeviceType> types) const noexcept {
    return not std::ranges::any_of(
        types, [this](vk::PhysicalDeviceType type) { return available_properties.properties.deviceType == type; });
  }

  bool is_discrete() const noexcept {
    return available_properties.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
  }

  bool is_integrated() const noexcept {
    return available_properties.properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu;
  }

  bool is_cpu() const noexcept {
    return available_properties.properties.deviceType == vk::PhysicalDeviceType::eCpu;
  }

  bool is_virtual() const noexcept {
    return available_properties.properties.deviceType == vk::PhysicalDeviceType::eVirtualGpu;
  }

  bool has_preset() const noexcept {
    assert(surface != nullptr && "To check for preset the surface should be set");
    for (auto i = 0u; i < available_queue_families.size(); i++) {
      auto preset_support = physical_device.getSurfaceSupportKHR(i, *surface);
      if (preset_support != 0)
        return true;
    }

    return false;
  }

  bool has_graphic_queue() const noexcept {
    return std::ranges::any_of(available_queue_families, [](const vk::QueueFamilyProperties2& queue) {
      return (queue.queueFamilyProperties.queueCount > 0) &&
             (queue.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics);
    });
  }

  bool has_compute_queue() const noexcept {
    return std::ranges::any_of(available_queue_families, [](const vk::QueueFamilyProperties2& queue) {
      return (queue.queueFamilyProperties.queueCount > 0) &&
             (queue.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute);
    });
  }

  bool has_transfer_queue() const noexcept {
    return std::ranges::any_of(available_queue_families, [](const vk::QueueFamilyProperties2& queue) {
      return (queue.queueFamilyProperties.queueCount > 0) &&
             (queue.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer);
    });
  }

  bool has_extension(std::string_view extension_name) const noexcept {
    auto match_extension = [extension_name](const vk::ExtensionProperties& ext) {
      return std::string_view{ext.extensionName} == extension_name;
    };

    return std::ranges::find_if(available_extensions, match_extension) != end(available_extensions);
  }

  bool has_extensions(std::span<std::string_view> requsted_extensions) const noexcept {
    return std::ranges::all_of(requsted_extensions,
                               [this](std::string_view extension_name) { return has_extension(extension_name); });
  }

  YAML::Node dump() const noexcept {
    return YAML::Clone(configuration);
  }

  PhysicalDevice& with_feature_10(vk::PhysicalDeviceFeatures2 required_feature) {
    auto& feat = feature_chain.get<vk::PhysicalDeviceFeatures2>();
    feat = combine(feat, required_feature);
    return *this;
  }

  PhysicalDevice& with_feature_11(vk::PhysicalDeviceVulkan11Features required_feature) {
    auto& feat = feature_chain.get<vk::PhysicalDeviceVulkan11Features>();
    feat = combine(feat, required_feature);
    return *this;
  }

  PhysicalDevice& with_feature_12(vk::PhysicalDeviceVulkan12Features required_feature) {
    auto& feat = feature_chain.get<vk::PhysicalDeviceVulkan12Features>();
    feat = combine(feat, required_feature);
    return *this;
  }

  PhysicalDevice& with_feature_13(vk::PhysicalDeviceVulkan13Features required_feature) {
    auto& feat = feature_chain.get<vk::PhysicalDeviceVulkan13Features>();
    feat = combine(feat, required_feature);
    return *this;
  }

  [[nodiscard]] Device create_device(const Instance& instance) const {
    auto queue_info_vec = std::vector<vk::DeviceQueueCreateInfo>{};
    for (auto i = 0u; i < available_queue_families.size(); i++) {
      auto priorities = std::vector<float>(available_queue_families[i].queueFamilyProperties.queueCount, 1.0f);
      queue_info_vec.emplace_back(vk::DeviceQueueCreateInfo{
          .queueFamilyIndex = i,
          .queueCount = available_queue_families[i].queueFamilyProperties.queueCount,
          .pQueuePriorities = priorities.data(),
      });
    }

    // ACHTUNG: the return type of the lambda has to be const char* since char[] will go out of scope
    auto layers = available_layers |
                  std::views::transform([](const auto& layer) -> const char* { return layer.layerName; }) |
                  std::ranges::to<std::vector<const char*>>();
    auto extensions =
        available_extensions |
        std::views::transform([](const auto& extension) -> const char* { return extension.extensionName; }) |
        std::ranges::to<std::vector<const char*>>();

    auto device_create_info = vk::DeviceCreateInfo{};
    device_create_info.pNext = &feature_chain.get<vk::PhysicalDeviceFeatures2>();
    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_info_vec.size());
    device_create_info.pQueueCreateInfos = queue_info_vec.data();
    device_create_info.enabledLayerCount = static_cast<uint32_t>(required_layers.size());
    device_create_info.ppEnabledLayerNames = required_layers.data();
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    device_create_info.ppEnabledExtensionNames = required_extensions.data();

    auto device = physical_device.createDevice(device_create_info);
    if (not device) {
      throw std::runtime_error{std::format("Unable to create a Vulkan Device: {}", vk::to_string(device.error()))};
    }

    VmaVulkanFunctions functions = {};
    functions.vkGetInstanceProcAddr = physical_device.getDispatcher()->vkGetInstanceProcAddr;
    functions.vkGetDeviceProcAddr = physical_device.getDispatcher()->vkGetDeviceProcAddr;

    auto allocator_info = VmaAllocatorCreateInfo{
        .flags = 0,
        .physicalDevice = static_cast<vk::raii::PhysicalDevice::CType>(
            static_cast<vk::raii::PhysicalDevice::CppType>(physical_device)),
        .device = static_cast<vk::raii::Device::CType>(static_cast<vk::raii::Device::CppType>(*device)),
        .preferredLargeHeapBlockSize = 0, // default
        .pAllocationCallbacks = nullptr,  //
        .pDeviceMemoryCallbacks = nullptr,
        .pHeapSizeLimit = nullptr,
        .pVulkanFunctions = &functions, //
        .instance = instance.native_handle(),
        .vulkanApiVersion = instance.get_api_version(),
        .pTypeExternalMemoryHandleTypes = nullptr,
    };

    VmaAllocator allocator;
    vmaCreateAllocator(&allocator_info, &allocator);

    return Device{allocator, std::move(*device)};
  }

private:
  explicit PhysicalDevice(NativeType device, std::vector<const char*> requirested_required_layers,
                          std::vector<const char*> requirested_required_extensions, Surface* surface = nullptr)
      : physical_device{device},
        surface{surface},
        required_layers(std::move(requirested_required_layers)),
        required_extensions{std::move(requirested_required_extensions)} {

    available_properties = physical_device.getProperties2();

    available_features = physical_device.getFeatures2();
    available_layers = physical_device.enumerateDeviceLayerProperties();
    available_extensions = physical_device.enumerateDeviceExtensionProperties();
    available_queue_families = physical_device.getQueueFamilyProperties2();

    for (auto i = 0u; i < available_queue_families.size(); i++) {
      auto queue_props = available_queue_families[i].queueFamilyProperties;
      if (queue_props.queueFlags & vk::QueueFlagBits::eGraphics) {
        available_graphic_queue_indexes.push_back(static_cast<uint32_t>(i));
      }
      if (queue_props.queueFlags & vk::QueueFlagBits::eTransfer) {
        available_transfer_queue_indexes.push_back(static_cast<uint32_t>(i));
      }
      if (queue_props.queueFlags & vk::QueueFlagBits::eCompute) {
        available_compute_queue_indexes.push_back(static_cast<uint32_t>(i));
      }
    }

    for (const auto& layer : available_layers) {
      auto layer_extension = physical_device.enumerateDeviceExtensionProperties(std::string{layer.layerName});
      available_extensions.insert(end(available_extensions), begin(layer_extension), end(layer_extension));
    }

    configuration["name"] = std::string_view{available_properties.properties.deviceName};
    configuration["device type"] = vk::to_string(available_properties.properties.deviceType);
    configuration["api version"] = vk_version_to_string(available_properties.properties.apiVersion);

#define ENUMERATE_FEATURE(feature)                                                                                     \
  {                                                                                                                    \
    YAML::Node node;                                                                                                   \
    node[#feature] = bool(available_features.features.feature);                                                        \
    configuration["features"].push_back(node);                                                                         \
  }

    ENUMERATE_FEATURE(robustBufferAccess);
    ENUMERATE_FEATURE(fullDrawIndexUint32);
    ENUMERATE_FEATURE(imageCubeArray);
    ENUMERATE_FEATURE(independentBlend);
    ENUMERATE_FEATURE(geometryShader);
    ENUMERATE_FEATURE(tessellationShader);
    ENUMERATE_FEATURE(sampleRateShading);
    ENUMERATE_FEATURE(dualSrcBlend);
    ENUMERATE_FEATURE(logicOp);
    ENUMERATE_FEATURE(multiDrawIndirect);
    ENUMERATE_FEATURE(drawIndirectFirstInstance);
    ENUMERATE_FEATURE(depthClamp);
    ENUMERATE_FEATURE(depthBiasClamp);
    ENUMERATE_FEATURE(fillModeNonSolid);
    ENUMERATE_FEATURE(depthBounds);
    ENUMERATE_FEATURE(wideLines);
    ENUMERATE_FEATURE(largePoints);
    ENUMERATE_FEATURE(alphaToOne);
    ENUMERATE_FEATURE(multiViewport);
    ENUMERATE_FEATURE(samplerAnisotropy);
    ENUMERATE_FEATURE(textureCompressionETC2);
    ENUMERATE_FEATURE(textureCompressionASTC_LDR);
    ENUMERATE_FEATURE(textureCompressionBC);
    ENUMERATE_FEATURE(occlusionQueryPrecise);
    ENUMERATE_FEATURE(pipelineStatisticsQuery);
    ENUMERATE_FEATURE(vertexPipelineStoresAndAtomics);
    ENUMERATE_FEATURE(fragmentStoresAndAtomics);
    ENUMERATE_FEATURE(shaderTessellationAndGeometryPointSize);
    ENUMERATE_FEATURE(shaderImageGatherExtended);
    ENUMERATE_FEATURE(shaderStorageImageExtendedFormats);
    ENUMERATE_FEATURE(shaderStorageImageMultisample);
    ENUMERATE_FEATURE(shaderStorageImageReadWithoutFormat);
    ENUMERATE_FEATURE(shaderStorageImageWriteWithoutFormat);
    ENUMERATE_FEATURE(shaderUniformBufferArrayDynamicIndexing);
    ENUMERATE_FEATURE(shaderSampledImageArrayDynamicIndexing);
    ENUMERATE_FEATURE(shaderStorageBufferArrayDynamicIndexing);
    ENUMERATE_FEATURE(shaderStorageImageArrayDynamicIndexing);
    ENUMERATE_FEATURE(shaderClipDistance);
    ENUMERATE_FEATURE(shaderCullDistance);
    ENUMERATE_FEATURE(shaderFloat64);
    ENUMERATE_FEATURE(shaderInt64);
    ENUMERATE_FEATURE(shaderInt16);
    ENUMERATE_FEATURE(shaderResourceResidency);

    for (const auto& layer : available_layers) {
      YAML::Node node;
      node["name"] = std::string_view{layer.layerName};
      node["description"] = (std::string_view{layer.description});
      node["spec version"] = vk_version_to_string(layer.specVersion);
      node["implementation version"] = layer.implementationVersion;
      configuration["layers"].push_back(node);
    }

    for (const auto& extension : available_extensions) {
      configuration["extension"].push_back(std::string_view{extension.extensionName});
    }

    configuration["required layers"] = required_layers;
    configuration["required extensions"] = required_extensions;

    auto queue_family_index = 0u;
    for (const auto& queue_family : available_queue_families) {
      YAML::Node node;
      node["flags"] = vk::to_string(queue_family.queueFamilyProperties.queueFlags);
      node["count"] = queue_family.queueFamilyProperties.queueCount;

      if (surface != nullptr) {
        auto preset_support = physical_device.getSurfaceSupportKHR(queue_family_index, *surface);
        node["preset support"] = static_cast<bool>(preset_support);
      }

      configuration["queue families"].push_back(node);

      ++queue_family_index;
    }
  }

  std::vector<VkPhysicalDeviceProperties2> get_available_properties() const noexcept {

    ;
    ;
  }

private:
  NativeType handle;
  Surface* surface;
  vk::PhysicalDeviceProperties2 available_properties;
  vk::PhysicalDeviceFeatures2 available_features;
  std::vector<vk::LayerProperties> available_layers;
  std::vector<vk::ExtensionProperties> available_extensions;
  std::vector<vk::QueueFamilyProperties2> available_queue_families;
  std::vector<const char*> required_layers;
  std::vector<const char*> required_extensions;
  std::vector<uint32_t> available_graphic_queue_indexes;
  std::vector<uint32_t> available_transfer_queue_indexes;
  std::vector<uint32_t> available_compute_queue_indexes;

  using DeviceFeatureChain = vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
                                                vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features>;
  DeviceFeatureChain feature_chain;

  YAML::Node configuration;
};
#endif
} // namespace vkh