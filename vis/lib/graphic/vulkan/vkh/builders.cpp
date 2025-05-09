module;

#include <cassert>
#include <volk.h>

#include "ecs/entity/handle.hpp"
#include "vk_enum_string_helper.h"

export module vis.graphic.vulkan.vkh:builders;

import std;
import vis.window;

import :traits;
import :concepts;
import :types;
import :constants;
import :helper;

export namespace vkh {

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

    std::ranges::sort(enabled_layer_names);
    auto [first_layer, last_layer] = std::ranges::unique(enabled_layer_names);
    enabled_layer_names.erase(first_layer, last_layer);

    std::ranges::sort(enabled_extension_names);
    auto [first_ext, last_ext] = std::ranges::unique(enabled_extension_names);
    enabled_extension_names.erase(first_ext, last_ext);

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

class PhysicalDeviceSurfaceInfo2Builder {
public:
  using NativeType = VkPhysicalDeviceSurfaceInfo2KHR;

  PhysicalDeviceSurfaceInfo2Builder(VkSurfaceKHR surface) noexcept {
    native = NativeType{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
        .pNext = nullptr,
        .surface = surface,
    };
  }

  PhysicalDeviceSurfaceInfo2Builder& with_next(VkBaseInStructure* next = nullptr) noexcept {
    native.pNext = next;
    return *this;
  }

  NativeType build() const noexcept {
    return native;
  }

private:
  NativeType native;
};

class VkSurfaceCapabilities2KHRBuilder {
public:
  using NativeType = VkSurfaceCapabilities2KHR;

  VkSurfaceCapabilities2KHRBuilder() noexcept {
    native = NativeType{
        .sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR,
        .pNext = nullptr,
        .surfaceCapabilities = {},
    };
  }

  VkSurfaceCapabilities2KHRBuilder& with_next(VkBaseInStructure* next = nullptr) noexcept {
    native.pNext = next;
    return *this;
  }

  NativeType build() const noexcept {
    return native;
  }

private:
  NativeType native;
};

class VkDeviceQueueCreateInfoBuilder {
public:
  using NativeType = VkDeviceQueueCreateInfo;

  VkDeviceQueueCreateInfoBuilder& with_next(void* required_next) {
    next = required_next;
    return *this;
  }

  VkDeviceQueueCreateInfoBuilder& with_family_index(std::size_t index) {
    family_index = static_cast<uint32_t>(index);
    return *this;
  }

  VkDeviceQueueCreateInfoBuilder& with_queue_counts(std::size_t count) {
    priorities.clear();
    std::fill_n(std::back_inserter(priorities), count, 1.0f);
    return *this;
  }

  VkDeviceQueueCreateInfoBuilder& with_queue_counts(std::span<float> priorities_hint) {
    priorities.insert(priorities.end(), begin(priorities_hint), end(priorities_hint));
    return *this;
  }

  NativeType build() const noexcept {
    // clang-format off
    return VkDeviceQueueCreateInfo {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = next,
      .flags = {},
      .queueFamilyIndex = family_index,
      .queueCount = static_cast<uint32_t>(priorities.size()),
      .pQueuePriorities = priorities.data(),
    };
    // clang-format on
  }

private:
  void* next = nullptr;
  uint32_t family_index{};
  std::vector<float> priorities{1.0f};
};

class VkDeviceCreateInfoBuilder {
public:
  using NativeType = VkDeviceCreateInfo;

  VkDeviceCreateInfoBuilder& with_next(void* required_next) {
    next = required_next;
    return *this;
  }

  VkDeviceCreateInfoBuilder& with_queue(const VkDeviceQueueCreateInfo& queue) noexcept {
    queues.push_back(queue);
    return *this;
  }

  VkDeviceCreateInfoBuilder& add_required_extension(const char* extension_name) noexcept {
    extensions.push_back(extension_name);
    return *this;
  }

  VkDeviceCreateInfoBuilder& add_required_extensions(std::span<const char*> required_extensions) noexcept {
    extensions.insert(extensions.end(), begin(required_extensions), end(required_extensions));
    return *this;
  }

  VkDeviceCreateInfo build() {
    // clang-format off
    return VkDeviceCreateInfo {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = next,
      .flags = {},
      .queueCreateInfoCount = static_cast<uint32_t>(queues.size()),
      .pQueueCreateInfos = queues.data(),
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data(),
      .pEnabledFeatures = &features      
    };
    // clang-format on
  }

private:
  void* next = nullptr;
  std::vector<const char*> layers;
  std::vector<const char*> extensions;
  std::vector<VkDeviceQueueCreateInfo> queues;
  std::vector<float> priorities;
  VkPhysicalDeviceFeatures features{};
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
  friend class PhysicalDeviceSelector;

public:
  using NativeType = VkDevice;

  explicit Device(std::nullptr_t) : handle{VK_NULL_HANDLE} /*, allocator{VK_NULL_HANDLE}, graphic_queue_index{}*/ {}

  Device(const Device& other) = delete;
  Device& operator=(const Device& other) = delete;

  Device(Device&& other) noexcept
      : handle{other.handle} /*, allocator{other.allocator}, graphic_queue_index{other.graphic_queue_index} */ {
    other.handle = VK_NULL_HANDLE;
    // other.allocator = nullptr;
  }

  Device& operator=(Device&& other) noexcept {
    std::swap(handle, other.handle);
    // std::swap(allocator, other.allocator);
    // std::swap(graphic_queue_index, other.graphic_queue_index);
    return *this;
  }

  ~Device() {
    if (handle != VK_NULL_HANDLE) {
      vkDestroyDevice(handle, nullptr);
      handle = VK_NULL_HANDLE;
    }
  }

  NativeType native_handle() const noexcept {
    return handle;
  }

private:
  Device(VkDevice device /*, VmaAllocator allocator, std::size_t graphic_queue_index*/)
      : handle{device} /*, allocator{allocator}, graphic_queue_index{graphic_queue_index*/
  {}

private:
  NativeType handle = VK_NULL_HANDLE;
  // VmaAllocator allocator = VK_NULL_HANDLE;
  // std::size_t graphic_queue_index = 0;
};

class PhysicalDevice {
  friend class PhysicalDeviceSelector;

public:
  using NativeType = VkPhysicalDevice;

  PhysicalDevice(std::nullptr_t) noexcept : handle{nullptr}, surface{nullptr} {}

  static VkPhysicalDeviceFeatures2 get_features2(VkPhysicalDevice device) noexcept {
    auto vk_features = PhysicalDeviceFeatures2Builder{}.build();
    vkGetPhysicalDeviceFeatures2(device, &vk_features);
    return vk_features;
  }

  static VkPhysicalDeviceProperties2 get_properties2(VkPhysicalDevice device) noexcept {
    auto vk_props = PhysicalDeviceProperties2Builder{}.build();
    vkGetPhysicalDeviceProperties2(device, &vk_props);
    return vk_props;
  }

  static std::vector<VkLayerProperties> get_device_layer_properties(VkPhysicalDevice device) noexcept {
    return enumerate<VkLayerProperties>(vkEnumerateDeviceLayerProperties, device);
  }

  static std::vector<VkLayerProperties> get_layers(VkPhysicalDevice device) noexcept {
    return enumerate<VkLayerProperties>(vkEnumerateDeviceLayerProperties, device);
  }

  static std::vector<VkExtensionProperties> get_extensions(VkPhysicalDevice device,
                                                           const char* layerName = nullptr) noexcept {
    return enumerate<VkExtensionProperties>(vkEnumerateDeviceExtensionProperties, device, layerName);
  }

  static std::vector<VkQueueFamilyProperties2> get_queue_famylies(VkPhysicalDevice device) noexcept {
    return enumerate<VkQueueFamilyProperties2>(vkGetPhysicalDeviceQueueFamilyProperties2, device);
  }

  static bool is_surface_supported(VkPhysicalDevice device, VkSurfaceKHR surface,
                                   std::size_t queue_family_index) noexcept {
    VkBool32 supported{};
    vkGetPhysicalDeviceSurfaceSupportKHR(device, static_cast<uint32_t>(queue_family_index), surface, &supported);
    return supported == VK_TRUE;
  }

  static VkSurfaceCapabilities2KHR get_surface_capabilities(VkPhysicalDevice device, VkSurfaceKHR surface) noexcept {
    auto physical_device_surface_info = PhysicalDeviceSurfaceInfo2Builder{surface}.build();
    VkSurfaceCapabilities2KHR result = VkSurfaceCapabilities2KHRBuilder{}.build();

    vkGetPhysicalDeviceSurfaceCapabilities2KHR(device, &physical_device_surface_info, &result);
    return result;
  }

  static std::vector<VkSurfaceFormat2KHR> get_surface_formats2(VkPhysicalDevice device, VkSurfaceKHR surface) noexcept {
    auto physical_device_surface_info = PhysicalDeviceSurfaceInfo2Builder{surface}.build();
    return enumerate<VkSurfaceFormat2KHR>(vkGetPhysicalDeviceSurfaceFormats2KHR, device, &physical_device_surface_info);
  }

  static std::vector<VkPresentModeKHR> get_present_modes(VkPhysicalDevice device, VkSurfaceKHR surface) noexcept {
    return enumerate<VkPresentModeKHR>(vkGetPhysicalDeviceSurfacePresentModesKHR, device, surface);
  }

  const VkPhysicalDeviceFeatures2& get_features2() const noexcept {
    return features;
  }

  const VkPhysicalDeviceProperties2& get_properties2() const noexcept {
    return properties;
  }

  const std::vector<VkLayerProperties>& get_layers() const noexcept {
    return available_layers;
  }

  const std::vector<VkExtensionProperties>& get_extensions() const noexcept {
    return available_extensions;
  }

  const std::vector<VkQueueFamilyProperties2>& get_queue_famylies() const noexcept {
    return available_queue_families;
  }

  bool is_surface_supported(std::size_t queue_family_index) const noexcept {
    auto it = surface_support_map.find(queue_family_index);
    if (it == end(surface_support_map))
      return false;

    return it->second;
  }

  const VkSurfaceCapabilities2KHR& get_surface_capabilities() const noexcept {
    return surface_capabilities;
  }

  const std::vector<VkSurfaceFormat2KHR>& get_surface_formats2() const noexcept {
    return surface_formats;
  }

  const std::vector<VkPresentModeKHR>& get_present_modes() const noexcept {
    return present_modes;
  }

  std::string device_name() const noexcept {
    return std::string{properties.properties.deviceName};
  }

  uint32_t device_api_version() const noexcept {
    return properties.properties.apiVersion;
  }

  VkPhysicalDeviceType get_device_type() const noexcept {
    return properties.properties.deviceType;
  }

  bool has_any(std::span<const PhysicalDeviceType> types) const noexcept {
    return std::ranges::any_of(types, [this](PhysicalDeviceType type) {
      return static_cast<PhysicalDeviceType>(properties.properties.deviceType) == type;
    });
  }

  bool has_not_any(const std::span<const PhysicalDeviceType> types) const noexcept {
    return not has_any(types);
  }

  bool is_type(PhysicalDeviceType type) const noexcept {
    return static_cast<PhysicalDeviceType>(properties.properties.deviceType) == type;
  }

  bool is_discrete() const noexcept {
    return is_type(PhysicalDeviceType::DiscreteGpu);
  }

  bool is_integrated() const noexcept {
    return is_type(PhysicalDeviceType::IntegratedGpu);
  }

  bool is_cpu() const noexcept {
    return is_type(PhysicalDeviceType::Cpu);
  }

  bool is_virtual() const noexcept {
    return is_type(PhysicalDeviceType::VirtualGpu);
  }

  bool has_preset() const noexcept {
    assert(surface != nullptr && "To check for preset the surface should be set");
    for (auto family_index = 0u; family_index < available_queue_families.size(); ++family_index)
      if (is_surface_supported(family_index))
        return true;
    return false;
  }

  bool has_graphic_queue() const noexcept {
    return has_queue(QueueFlagBits::Graphics);
  }

  bool has_compute_queue() const noexcept {
    return has_queue(QueueFlagBits::Compute);
  }

  bool has_transfer_queue() const noexcept {
    return has_queue(QueueFlagBits::Transfer);
  }

  bool has_queue(QueueFlags flags) const noexcept {
    return std::ranges::any_of(available_queue_families, [flags](const VkQueueFamilyProperties2& queue) {
      return (queue.queueFamilyProperties.queueCount > 0) &&
             (flags & static_cast<QueueFlags>(queue.queueFamilyProperties.queueFlags));
    });
  }

  bool has_extension(std::string_view extension_name) const noexcept {
    auto match_extension = [extension_name](const VkExtensionProperties& ext) {
      return std::string_view{ext.extensionName} == extension_name;
    };

    return std::ranges::find_if(available_extensions, match_extension) != end(available_extensions);
  }

  std::optional<std::size_t> get_first_graphic_and_present_queue_family_index() const noexcept {
    for (std::size_t i = 0u; i < available_queue_families.size(); ++i) {
      const auto& queue = available_queue_families[i].queueFamilyProperties;
      if (queue.queueCount > 0 && (queue.queueFlags & static_cast<VkQueueFlags>(QueueFlagBits::Graphics)) &&
          is_surface_supported(i))
        return i;
    }

    return std::nullopt;
  }

  template <std::ranges::range R>
    requires std::convertible_to<std::ranges::range_value_t<R>, std::string_view>
  bool has_extensions(R requested_extensions) const noexcept {
    return std::ranges::all_of(requested_extensions,
                               [this](std::string_view extension_name) { return has_extension(extension_name); });
  }

  NativeType native_handle() const noexcept {
    return handle;
  }

#if 0
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
#endif

private:
  PhysicalDevice(NativeType device, Surface* surface) noexcept : handle{device}, surface{surface} {
    init_features2();
    init_properties2();
    init_layers();
    init_extensions();
    init_queue_famylies();
    init_surface_support_map();
    init_surface_capabilities();
    init_surface_formats2();
    init_present_modes();
  }

  void init_features2() noexcept {
    features = PhysicalDevice::get_features2(handle);
  }

  void init_properties2() noexcept {
    properties = PhysicalDevice::get_properties2(handle);
  }

  void init_layers() noexcept {
    available_layers = PhysicalDevice::get_layers(handle);
  }

  void init_extensions() noexcept {
    available_extensions = PhysicalDevice::get_extensions(handle);

    for (const auto& layer : available_layers) {
      auto extension_for_layer = PhysicalDevice::get_extensions(handle, layer.layerName);
      available_extensions.insert(end(available_extensions), begin(extension_for_layer), end(extension_for_layer));
    }
  }

  void init_queue_famylies() noexcept {
    available_queue_families = PhysicalDevice::get_queue_famylies(handle);
  }

  void init_surface_support_map() noexcept {
    assert(surface != nullptr && "You mush assign the surface before");
    for (auto i = 0u; i < available_queue_families.size(); ++i) {
      surface_support_map[i] = PhysicalDevice::is_surface_supported(handle, surface->native_handle(), i);
    }
  }

  void init_surface_capabilities() noexcept {
    assert(surface != nullptr && "You mush assign the surface before");
    surface_capabilities = PhysicalDevice::get_surface_capabilities(handle, surface->native_handle());
  }

  void init_surface_formats2() noexcept {
    assert(surface != nullptr && "You mush assign the surface before");
    surface_formats = PhysicalDevice::get_surface_formats2(handle, surface->native_handle());
  }

  void init_present_modes() noexcept {
    assert(surface != nullptr && "You mush assign the surface before");
    present_modes = PhysicalDevice::get_present_modes(handle, surface->native_handle());
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

    result += "    features:\n";
#define ENUMERATE_FEATURE(feature) result += std::format("      {}: {}\n", #feature, bool(features.features.feature));
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
#undef ENUMERATE_FEATURE

    result += "    queue families:\n";
    for (auto i = 0u; i < available_queue_families.size(); ++i) {
      const auto& queue_family = available_queue_families[i];
      result += std::format("      - flags: {}\n"
                            "        count: {}\n"
                            "        present support: {}\n",
                            string_VkQueueFlags(queue_family.queueFamilyProperties.queueFlags),
                            queue_family.queueFamilyProperties.queueCount, is_surface_supported(i));
    }

    result += std::format(
        "    surface capabilities:\n"
        "      min image count: {}\n"
        "      max image count: {}\n"
        "      current image extent: {}x{}\n"
        "      min image extent: {}x{}\n"
        "      max image extent: {}x{}\n"
        "      max array layers: {}\n"
        "      supported transforms: {}\n"
        "      current transforms: {}\n"
        "      supported composite alpha: {}\n"
        "      supported usage flags: {}\n",
        surface_capabilities.surfaceCapabilities.minImageCount, surface_capabilities.surfaceCapabilities.maxImageCount,
        surface_capabilities.surfaceCapabilities.currentExtent.width,
        surface_capabilities.surfaceCapabilities.currentExtent.height,
        surface_capabilities.surfaceCapabilities.minImageExtent.width,
        surface_capabilities.surfaceCapabilities.minImageExtent.height,
        surface_capabilities.surfaceCapabilities.maxImageExtent.width,
        surface_capabilities.surfaceCapabilities.maxImageExtent.height,
        surface_capabilities.surfaceCapabilities.maxImageArrayLayers,
        string_VkSurfaceTransformFlagsKHR(surface_capabilities.surfaceCapabilities.supportedTransforms),
        string_VkSurfaceTransformFlagBitsKHR(surface_capabilities.surfaceCapabilities.currentTransform),
        string_VkCompositeAlphaFlagsKHR(surface_capabilities.surfaceCapabilities.supportedCompositeAlpha),
        string_VkImageUsageFlags(surface_capabilities.surfaceCapabilities.supportedUsageFlags));

    result += "    surface formats:\n";
    for (const VkSurfaceFormat2KHR& surface_format : surface_formats) {
      result += std::format("      - format: {}\n"
                            "        color space: {}\n",
                            string_VkFormat(surface_format.surfaceFormat.format),
                            string_VkColorSpaceKHR(surface_format.surfaceFormat.colorSpace));
    }

    result += "    present modes:\n";
    for (const VkPresentModeKHR& present_mode : present_modes) {
      result += std::format("      - {}\n", string_VkPresentModeKHR(present_mode));
    }

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
  std::map<std::size_t, bool> surface_support_map;
  VkSurfaceCapabilities2KHR surface_capabilities;

  std::vector<VkSurfaceFormat2KHR> surface_formats;
  std::vector<VkPresentModeKHR> present_modes;
};

class PhysicalDeviceSelector {
public:
  explicit PhysicalDeviceSelector(Instance& instance, Surface* surface = nullptr)
      : instance{instance}, surface{surface} {}

  PhysicalDeviceSelector& add_required_extensions(std::span<const char*> extensions) {
    required_gpu_extensions.insert(std::end(required_gpu_extensions), std::begin(extensions), std::end(extensions));
    device_create_info_builder.add_required_extensions(extensions);
    return *this;
  }

  PhysicalDeviceSelector& add_required_extensions(const char* extension) {
    required_gpu_extensions.push_back(extension);
    device_create_info_builder.add_required_extension(extension);
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

  PhysicalDeviceSelector& allow_gpu_type(PhysicalDeviceType type) {
    allowed_gpu_types.emplace_back(type);
    return *this;
  }

  PhysicalDeviceSelector& set_require_preset() {
    require_preset_queue = false;
    return *this;
  }

  PhysicalDeviceSelector& set_require_graphic_queue() {
    require_graphic_queue = false;
    return *this;
  }

  PhysicalDeviceSelector& with_queue(const VkDeviceQueueCreateInfo& queue) noexcept {
    device_create_info_builder.with_queue(queue);
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

  using PhysicalDeviceVec = std::vector<PhysicalDevice>;
  using PhysicalDeviceConstIterator = PhysicalDeviceVec::const_iterator;
  using PhysicalDeviceIterator = PhysicalDeviceVec::iterator;

  PhysicalDeviceIterator select(PhysicalDeviceIterator first, PhysicalDeviceIterator last) const noexcept {
    auto physical_devices = std::ranges::subrange(first, last);

    // clang-format off
    auto scores = physical_devices
      | std::views::transform([this](const auto& device) -> int {
                        return score_device(device);
                      })
      | std::ranges::to<std::vector<int>>();
    // clang-format on
    // clang-format on

    auto selected_device_iter = std::ranges::max_element(scores);
    auto selected_device_idx = std::distance(begin(scores), selected_device_iter);
    return first + selected_device_idx;
  }

  Device create_device(const PhysicalDevice& physical_device) {
    auto device_create_info = device_create_info_builder.build();
    VkDevice device;
    vkCreateDevice(physical_device.native_handle(), &device_create_info, nullptr, &device);

    std::call_once(device_initialize, [device]() { volkLoadDevice(device); });
    return Device{device};
  }

private:
  bool is_suitable(const PhysicalDevice& device) const noexcept {
    if (not device.has_any(allowed_gpu_types))
      return false;

    if (require_graphic_queue and not device.has_graphic_queue())
      return false;

    if (require_preset_queue and not device.has_preset())
      return false;

    if (not device.has_extensions(required_gpu_extensions))
      return false;

    // TODO: check the queue indexes are correct and match the type

    return true;
  }

  int score_device(const PhysicalDevice& device) const noexcept {
    if (not is_suitable(device))
      return 0;

    int score = 0;
    if (device.is_discrete()) {
      score += 10000;
    }
    if (device.is_integrated()) {
      score += 1000;
    }
    if (device.is_cpu()) {
      score += 100;
    }
    return score;
  }

private:
  Instance& instance;
  Surface* surface = nullptr;

  std::vector<const char*> required_gpu_extensions;
  std::vector<const char*> required_gpu_layers;
  std::vector<PhysicalDeviceType> allowed_gpu_types;
  bool require_preset_queue{true};
  bool require_graphic_queue{true};

  VkDeviceCreateInfoBuilder device_create_info_builder;
  std::once_flag device_initialize;
};

class Semaphore {
  friend class SemaphoreBuilder;

public:
  using NativeHandle = VkSemaphore;

  Semaphore(std::nullptr_t) noexcept : handle{VK_NULL_HANDLE}, device{nullptr} {}

  Semaphore(const Semaphore&) = delete;
  Semaphore& operator=(const Semaphore&) = delete;

  Semaphore(Semaphore&& other) : handle{other.handle}, device{other.device} {
    other.handle = VK_NULL_HANDLE;
    other.device = nullptr;
  }

  Semaphore& operator=(Semaphore&& other) noexcept {
    std::swap(handle, other.handle);
    std::swap(device, other.device);
    return *this;
  }

  ~Semaphore() {
    if (handle != VK_NULL_HANDLE) {
      vkDestroySemaphore(device->native_handle(), handle, nullptr);
      handle = VK_NULL_HANDLE;
    }
  }

  NativeHandle native_handle() const noexcept {
    return handle;
  }

private:
  Semaphore(NativeHandle handle, Device* device) : handle{handle}, device{device} {}

private:
  NativeHandle handle = VK_NULL_HANDLE;
  Device* device = nullptr;
};

class SemaphoreBuilder {
public:
  explicit SemaphoreBuilder(Device& device) : device{device} {
    // clang-format off
    semaphore_create_info = VkSemaphoreCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = {},
    };
    // clang-format on
  }

  SemaphoreBuilder& with_next(void* next) {
    semaphore_create_info.pNext = next;
    return *this;
  }

  Semaphore build() const noexcept {
    VkSemaphore semaphore;
    vkCreateSemaphore(device.native_handle(), &semaphore_create_info, nullptr, &semaphore);
    return Semaphore{semaphore, &device};
  }

private:
  Device& device;
  VkSemaphoreCreateInfo semaphore_create_info;
};

class Swapchain {
  friend class SwapchainBuilder;

public:
  using NativeHandle = VkSwapchainKHR;

  Swapchain(std::nullptr_t) noexcept : handle{VK_NULL_HANDLE}, device{nullptr} {}

  Swapchain(const Swapchain&) = delete;
  Swapchain& operator=(const Swapchain&) = delete;

  Swapchain(Swapchain&& other) : handle{other.handle}, device{other.device} {
    other.handle = VK_NULL_HANDLE;
    other.device = nullptr;
  }

  Swapchain& operator=(Swapchain&& other) noexcept {
    std::swap(handle, other.handle);
    std::swap(device, other.device);
    return *this;
  }

  ~Swapchain() {
    if (handle != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(device->native_handle(), handle, nullptr);
      handle = VK_NULL_HANDLE;
    }
  }

  NativeHandle native_handle() const noexcept {
    return handle;
  }

private:
  Swapchain(NativeHandle handle, Device* device) : handle{handle}, device{device} {}

private:
  NativeHandle handle = VK_NULL_HANDLE;
  Device* device = nullptr;
};

class SwapchainBuilder {
public:
  SwapchainBuilder(PhysicalDevice& physical_device, Device& device, Surface& surface) : device{device} {
    auto surface_caps = physical_device.get_surface_capabilities();
    // clang-format off
    swapchain_create_info = VkSwapchainCreateInfoKHR{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = {},
      .surface = surface.native_handle(),
      .minImageCount = surface_caps.surfaceCapabilities.minImageCount,
      .imageFormat = {},
      .imageColorSpace = {},
      .imageExtent = surface_caps.surfaceCapabilities.currentExtent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = {},
      .pQueueFamilyIndices = nullptr,
      .preTransform = surface_caps.surfaceCapabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = VK_PRESENT_MODE_MAILBOX_KHR,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE
    };
    // clang-format on
  }

  SwapchainBuilder& with_required_format(Format format) {
    required_format = format;
    return *this;
  }

  SwapchainBuilder& with_present_mode(PresentMode mode) {
    swapchain_create_info.presentMode = static_cast<VkPresentModeKHR>(mode);
    return *this;
  }

  SwapchainBuilder& with_image_count(int image_count) {
    swapchain_create_info.minImageCount = static_cast<uint32_t>(image_count);
    return *this;
  }

  SwapchainBuilder& with_extent(int width, int height) {
    swapchain_create_info.imageExtent = VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    return *this;
  }

  SwapchainBuilder& with_usage(ImageUsageFlags required_image_usage) {
    image_usage_bit |= required_image_usage;
    return *this;
  }

  SwapchainBuilder& add_queue_family_index(std::size_t index) {
    queue_family_indices.push_back(static_cast<uint32_t>(index));
    return *this;
  }

  SwapchainBuilder& with_old_swapchain(Swapchain& old_swapchain) {
    swapchain_create_info.oldSwapchain = old_swapchain.native_handle();
    return *this;
  }

  Swapchain build() {
    if (required_format) {
      swapchain_create_info.imageFormat = static_cast<VkFormat>(*required_format);
    }

    swapchain_create_info.imageUsage = static_cast<VkImageUsageFlags>(image_usage_bit);

    swapchain_create_info.queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices.size());
    swapchain_create_info.pQueueFamilyIndices = queue_family_indices.data();

    VkSwapchainKHR swapchain;
    vkCreateSwapchainKHR(device.native_handle(), &swapchain_create_info, nullptr, &swapchain);

    return Swapchain{swapchain, &device};
  }

private:
  Device& device;
  VkSwapchainCreateInfoKHR swapchain_create_info;
  std::optional<uint32_t> min_image_count{};
  ImageUsageFlags image_usage_bit = ImageUsageFlagBits::ColorAttachment;
  std::vector<uint32_t> queue_family_indices;
  std::optional<Format> required_format;
};

class CommandPool {
  friend class CommandPoolBuilder;

public:
  using NativeHandle = VkCommandPool;

  CommandPool(std::nullptr_t) : handle{VK_NULL_HANDLE}, device{nullptr} {}

  CommandPool(const CommandPool&) = delete;
  CommandPool& operator=(const CommandPool&) = delete;

  CommandPool(CommandPool&& other) noexcept : handle{other.handle}, device{other.device} {
    other.handle = VK_NULL_HANDLE;
    other.device = nullptr;
  }

  CommandPool& operator=(CommandPool&& other) noexcept {
    std::swap(handle, other.handle);
    std::swap(device, other.device);
    return *this;
  }

  ~CommandPool() {
    if (handle != VK_NULL_HANDLE) {
      vkDestroyCommandPool(device->native_handle(), handle, nullptr);
      handle = VK_NULL_HANDLE;
      device = nullptr;
    }
  }

  NativeHandle native_handle() const noexcept {
    return handle;
  }

private:
  CommandPool(NativeHandle handle, Device* device) : handle{handle}, device{device} {}

private:
  NativeHandle handle = VK_NULL_HANDLE;
  Device* device = nullptr;
};

class CommandPoolBuilder {
public:
  explicit CommandPoolBuilder(Device& device) : device{device} {
    // clang-format off
    command_pool_create_info = VkCommandPoolCreateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = {},
    };
    // clang-format on
  }

  CommandPoolBuilder& with_next(void* next) {
    command_pool_create_info.pNext = next;
    return *this;
  }

  CommandPoolBuilder& with_queue_family_index(std::size_t index) {
    command_pool_create_info.queueFamilyIndex = static_cast<uint32_t>(index);
    return *this;
  }

  CommandPoolBuilder& with_flags(CommandPoolCreateFlags required_flags) {
    command_pool_create_info.flags = static_cast<VkCommandPoolCreateFlags>(required_flags);
    return *this;
  }

  CommandPool build() const noexcept {
    VkCommandPool command_pool;
    vkCreateCommandPool(device.native_handle(), &command_pool_create_info, nullptr, &command_pool);

    return CommandPool{command_pool, &device};
  }

private:
  VkCommandPoolCreateInfo command_pool_create_info;
  CommandPoolCreateFlags flags;
  Device& device;
};

} // namespace vkh