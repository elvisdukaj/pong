module;

#include <cassert>
#include <volk.h>

export module vis.graphic.vulkan.vkh;

export import std;

namespace helper {

template <std::integral T> constexpr uint32_t apiVersionMajor(T const version) {
  return (static_cast<uint32_t>(version) >> 22U) & 0x7FU;
}

template <std::integral T> constexpr uint32_t apiVersionMinor(T const version) {
  return (static_cast<uint32_t>(version) >> 12U) & 0x3FFU;
}

template <std::integral T> constexpr uint32_t apiVersionPatch(T const version) {
  return static_cast<uint32_t>(version) & 0xFFFU;
}

template <std::integral T> constexpr uint32_t apiVersionVariant(T const version) {
  return static_cast<uint32_t>(version) >> 29U;
}

template <std::integral T>
constexpr uint32_t makeApiVersion(T const variant, T const major, T const minor, T const patch) {
  return ((((uint32_t)(variant)) << 29U) | (((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) |
          ((uint32_t)(patch)));
}

template <std::integral T> constexpr uint32_t makeVersion(T const major, T const minor, T const patch) {
  return ((((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch)));
}

std::string vk_version_to_string(uint32_t version) noexcept {
  return std::format("{}.{}.{}", apiVersionMajor(version), apiVersionMinor(version), apiVersionPatch(version));
}

} // namespace helper

export namespace vis::vkh {

// Extensions names
constexpr const char* KHRGetPhysicalDeviceProperties2ExtensionName =
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
constexpr const char* KHRGetSurfaceCapabilities2ExtensionName = VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME;
#if defined(VK_USE_PLATFORM_METAL_EXT)
constexpr const char* EXTMetalSurfaceExtensionName = VK_EXT_METAL_SURFACE_EXTENSION_NAME;
#endif
constexpr const char* KHRPortabilityEnumerationExtensionName = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;

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
    application_version = helper::makeApiVersion(variant, maj, min, patch);
    return *this;
  }

  ApplicationInfoBuilder& with_application_version(uint32_t maj, uint32_t min, uint32_t patch) noexcept {
    application_version = helper::makeVersion(maj, min, patch);
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
    application_version = helper::makeApiVersion(variant, maj, min, patch);
    return *this;
  }

  ApplicationInfoBuilder& with_engine_version(uint32_t maj, uint32_t min, uint32_t patch) noexcept {
    application_version = helper::makeVersion(maj, min, patch);
    return *this;
  }

  ApplicationInfoBuilder& with_api_version(uint32_t requested_api_version) noexcept {
    api_version = requested_api_version;
    return *this;
  }

  ApplicationInfoBuilder& with_api_version(uint32_t variant, uint32_t maj, uint32_t min, uint32_t patch) noexcept {
    application_version = helper::makeApiVersion(variant, maj, min, patch);
    return *this;
  }

  ApplicationInfoBuilder& with_api_version(uint32_t maj, uint32_t min, uint32_t patch) noexcept {
    application_version = helper::makeVersion(maj, min, patch);
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

class Context {
public:
  Context() {
    api_version = volkGetInstanceVersion();
    available_layers = get_available_layers();
    available_extensions = get_available_extensions();

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

  std::vector<VkLayerProperties> get_available_layers() noexcept {
    std::vector<VkLayerProperties> properties;
    uint32_t propertyCount;
    Result result;
    do {
      result = static_cast<Result>(vkEnumerateInstanceLayerProperties(&propertyCount, nullptr));
      if ((result == Result::Success) && propertyCount) {
        properties.resize(propertyCount);
        result = static_cast<Result>(vkEnumerateInstanceLayerProperties(
            &propertyCount, reinterpret_cast<VkLayerProperties*>(properties.data())));
      }
    } while (result == Result::Incomplete);
    // VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VULKAN_HPP_NAMESPACE_STRING
    // "::Context::enumerateInstanceExtensionProperties");
    // VULKAN_HPP_ASSERT(propertyCount <= properties.size());
    if (propertyCount < properties.size()) {
      properties.resize(propertyCount);
    }
    return properties;
  }

  std::vector<VkExtensionProperties>
  get_available_extensions(std::optional<std::string_view> layer_name = std::nullopt) {
    std::vector<VkExtensionProperties> properties;
    uint32_t propertyCount;
    Result result;
    do {
      result = static_cast<Result>(
          vkEnumerateInstanceExtensionProperties(layer_name ? layer_name->data() : nullptr, &propertyCount, nullptr));
      if ((result == Result::Success) && propertyCount) {
        properties.resize(propertyCount);
        result = static_cast<Result>(
            vkEnumerateInstanceExtensionProperties(layer_name ? layer_name->data() : nullptr, &propertyCount,
                                                   reinterpret_cast<VkExtensionProperties*>(properties.data())));
      }
    } while (result == Result::Incomplete);
    // VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VULKAN_HPP_NAMESPACE_STRING
    // "::Context::enumerateInstanceExtensionProperties");
    // VULKAN_HPP_ASSERT(propertyCount <= properties.size());
    if (propertyCount < properties.size()) {
      properties.resize(propertyCount);
    }
    return properties;
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
    minimum_instance_version = helper::makeApiVersion(variant, major, minor, patch);
    return *this;
  }

  InstanceBuilder& with_maximum_required_instance_version(int variant, int major, int minor, int patch) noexcept {
    maximum_instance_version = helper::makeApiVersion(variant, major, minor, patch);
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

  uint32_t minimum_instance_version = helper::makeApiVersion(0, 1, 0, 0);
  uint32_t maximum_instance_version = helper::makeApiVersion(0, 1, 0, 0);

  ApplicationInfoBuilder app_info_builder;
  InstanceCreateInfoBuilder instance_create_info_builder;

  // Custom allocator
  // std::optional<vk::AllocationCallbacks> allocation_callbacks = std::nullopt;
};
} // namespace vis::vkh