module;

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <yaml-cpp/yaml.h>

#include <cassert>

export module vkh;

import std;
import vulkan_hpp;
import vis.window;

export namespace vkh {

// forward and using
class Context;
class InstanceBuilder;
class Instance;
// using Surface = ::vk::raii::SurfaceKHR;
class Surface;
class PhysicalDeviceSelector;
using ::vk::CommandPool;
class Device;
using ::vk::raii::RenderPass;
class RenderPassBuilder;

constexpr std::vector<const char*> get_physical_device_extensions() {
	std::vector<const char*> required_extensions = {};

#if defined(__APPLE__)
	required_extensions.push_back("VK_KHR_portability_subset");
#endif
	return required_extensions;
}

std::string vk_version_to_string(uint32_t version) {
	return std::format("{}.{}.{}", vk::apiVersionMajor(version), vk::apiVersionMinor(version),
										 vk::apiVersionPatch(version));
}

std::vector<const char*> get_required_extensions() {
	std::vector<const char*> required_extensions;

#if defined(__APPLE__)
	required_extensions.push_back(vk::EXTMetalSurfaceExtensionName);
	required_extensions.push_back(vk::KHRPortabilityEnumerationExtensionName);
#elif defined(__linux__)
	// required_extensions.push_back(vk::)
#endif

	return required_extensions;
}

std::vector<const char*> get_required_layers() {
	std::vector<const char*> required_layers;

#if not defined(NDEBUG)
	required_layers.push_back("VK_LAYER_KHRONOS_validation");

#if not defined(__linux)
	required_layers.push_back("VK_LAYER_LUNARG_api_dump");
#endif

#endif

	return required_layers;
}

constexpr ::vk::InstanceCreateFlags get_required_instance_flags() {
	vk::InstanceCreateFlags flags;

#if defined(__APPLE__)
	// flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
	flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif
	return flags;
}

class Context {
	friend class InstanceBuilder;

public:
	Context() {
		api_version = context.enumerateInstanceVersion();
		config_["api_version"] = vk_version_to_string(api_version);

		available_extensions = context.enumerateInstanceExtensionProperties();
		available_layers = context.enumerateInstanceLayerProperties();

		for (const auto& layer : available_layers) {
			YAML::Node layer_conf;
			layer_conf["name"] = std::string_view{layer.layerName};
			layer_conf["description"] = std::string_view{layer.description};
			layer_conf["implementation version"] = layer.implementationVersion;
			layer_conf["spec version"] = vk_version_to_string(layer.specVersion);
			config_["available layers"].push_back(layer_conf);

			auto layer_extensions = context.enumerateInstanceExtensionProperties(std::string{layer.layerName});
			available_extensions.insert(end(available_extensions), begin(layer_extensions), end(layer_extensions));
		}

		YAML::Node node = config_["available extensions"];
		for (const auto& extension : available_extensions) {
			node.push_back(std::string_view{extension.extensionName});
		}
	}

	uint32_t api_version;

	std::vector<const char*> get_available_layers() const {
		// clang-format off
		return available_layers
			| std::views::transform([](const auto& layer) -> const char* { return layer.layerName; })
			| std::ranges::to<std::vector<const char*>>();
		// clang-format on
	};

	std::vector<const char*> get_available_extensions() const {
		// clang-format off
		return available_extensions
			| std::views::transform([](const auto& extension) -> const char* { return extension.extensionName; })
			| std::ranges::to<std::vector<const char*>>();
		// clang-format on
	}

	bool has_extension(std::string_view extension_name) const {
		auto match_extension_name = [extension_name](const vk::ExtensionProperties& extension) -> bool {
			return std::string_view{extension.extensionName} == extension_name;
		};

		auto it = std::find_if(begin(available_extensions), end(available_extensions), match_extension_name);
		return it != end(available_extensions);
	}

	bool has_extensions(std::span<const char*> extensions) {
		return std::all_of(begin(extensions), end(extensions),
											 [this](const char* extension_name) { return has_extension(extension_name); });
	}

	bool has_layer(std::string_view layer_name) const {
		auto match_layer_name = [layer_name](const vk::LayerProperties& layer) -> bool {
			return std::string_view{layer.layerName} == layer_name;
		};

		auto it = std::find_if(begin(available_layers), end(available_layers), match_layer_name);
		return it != end(available_layers);
	}

	bool has_layers(std::span<const char*> layers) {
		return std::all_of(begin(layers), end(layers), [this](const char* layer_name) { return has_layer(layer_name); });
	}

	YAML::Node serialize() const {
		return config_;
	}

private:
	vk::raii::Context context;

	std::vector<vk::ExtensionProperties> available_extensions;
	std::vector<vk::LayerProperties> available_layers;
	YAML::Node config_;
};

class Instance : public vk::raii::Instance {
public:
	using vk::raii::Instance::CType;
	using vk::raii::Instance::Instance;

	Instance(vk::raii::Instance&& instance, uint32_t required_version)
			: vk::raii::Instance{std::move(instance)}, api_version{required_version} {
		config["version"] = vk_version_to_string(api_version);
	}

	YAML::Node dump() const {
		return YAML::Clone(config);
	}

	void swap(Instance& other) noexcept {
		std::swap(static_cast<vk::raii::Instance&>(*this), static_cast<vk::raii::Instance&>(other));
		std::swap(api_version, other.api_version);
		config = YAML::Clone(other.config);
	}

	Instance(const Instance& other) = delete;
	Instance& operator=(const Instance& other) = delete;

	Instance(Instance&& other) : vk::raii::Instance{nullptr}, api_version{0} {
		swap(other);
	}

	Instance& operator=(Instance&& other) {
		swap(other);
		return *this;
	}

	uint32_t get_api_version() const noexcept {
		return api_version;
	}

	CType native_handle() const noexcept {
		return static_cast<CType>(static_cast<CppType>(*this));
	}

	using vk::raii::Instance::enumeratePhysicalDevices;

private:
	uint32_t api_version;
	YAML::Node config;
};

class InstanceBuilder {
public:
	InstanceBuilder(Context& context) : context{context} {}

	InstanceBuilder& with_minimum_required_instance_version(int variant, int major, int minor, int patch) {
		minimim_instance_version = vk::makeApiVersion(variant, major, minor, patch);
		return *this;
	}

	InstanceBuilder& with_maximum_required_instance_version(int variant, int major, int minor, int patch) {
		maximum_instance_version = vk::makeApiVersion(variant, major, minor, patch);
		return *this;
	}

	InstanceBuilder& with_app_name(std::string_view name) {
		app_name = name;
		return *this;
	}

	InstanceBuilder& with_app_version(uint32_t version) {
		app_version = version;
		return *this;
	}

	InstanceBuilder& with_app_version(int major, int minor, int patch) {
		app_version = vk::makeApiVersion(0, major, minor, patch);
		return *this;
	}

	InstanceBuilder& with_engine_name(std::string_view name) {
		engine_name = name;
		return *this;
	}

	InstanceBuilder& with_engine_version(uint32_t version) {
		engine_version = version;
		return *this;
	}

	InstanceBuilder& with_engine_version(int major, int minor, int patch) {
		engine_version = vk::makeApiVersion(0, major, minor, patch);
		return *this;
	}

	InstanceBuilder& add_required_layer(const char* layer_name) {
		required_layers.push_back(layer_name);
		return *this;
	}

	InstanceBuilder& add_required_layers(std::span<const char*> layers) {
		required_layers.insert(required_layers.end(), begin(layers), end(layers));
		return *this;
	}

	InstanceBuilder& add_required_extension(const char* extension) {
		required_extensions.push_back(extension);
		return *this;
	}

	InstanceBuilder& add_required_extensions(std::span<const char*> extensions) {
		required_extensions.insert(required_extensions.end(), begin(extensions), end(extensions));
		return *this;
	}

	InstanceBuilder& with_app_flags(vk::InstanceCreateFlags instance_flag) {
		flags = instance_flag;
		return *this;
	}

	Instance build() {
		maximum_instance_version = std::min(maximum_instance_version, context.api_version);
		if (minimim_instance_version > maximum_instance_version) {
			throw std::runtime_error{std::format("The minimum vulkan version is {} bu the minimum required is {}",
																					 vk_version_to_string(maximum_instance_version),
																					 vk_version_to_string(minimim_instance_version))};
		}

		auto required_api_version = std::max(minimim_instance_version, maximum_instance_version);

		auto avilable_layers = context.get_available_layers();
		if (not context.has_layers(required_layers)) {
			throw std::runtime_error("Some required layers are not avilable");
		}

		auto avilable_extensions = context.get_available_extensions();
		if (not context.has_extensions(required_extensions)) {
			throw std::runtime_error("Some required extensions are not avilable");
		}

		auto app_info = vk::ApplicationInfo{
				.pNext = nullptr,
				.pApplicationName = app_name.c_str(),
				.applicationVersion = app_version,
				.pEngineName = engine_name.c_str(),
				.engineVersion = engine_version,
				.apiVersion = required_api_version,
		};

		vk::InstanceCreateInfo create_info{
				.flags = flags,
				.pApplicationInfo = &app_info,
				.enabledLayerCount = static_cast<uint32_t>(size(required_layers)),
				.ppEnabledLayerNames = required_layers.data(),
				.enabledExtensionCount = static_cast<uint32_t>(size(required_extensions)),
				.ppEnabledExtensionNames = required_extensions.data(),
		};

		auto instance = context.context.createInstance(create_info);
		if (not instance) {
			throw std::runtime_error{std::format("Unable to create a Vulkan Instance: {}", vk::to_string(instance.error()))};
		}

		return Instance(std::move(*instance), required_api_version);
	}

private:
	Context& context;

	uint32_t minimim_instance_version = vk::makeApiVersion(0, 1, 0, 0);
	uint32_t maximum_instance_version = vk::makeApiVersion(0, 1, 0, 0);

	// VkApplicationInfo
	std::string app_name = "";
	std::string engine_name = "vis game engine";
	uint32_t app_version = 0;
	uint32_t engine_version = vk::makeApiVersion(0, 0, 0, 1);
	// VLInstanceCreateInfo
	std::vector<const char*> required_layers;
	std::vector<const char*> required_extensions;
	vk::InstanceCreateFlags flags;
	std::vector<vk::BaseOutStructure> nexts;

	// Custom allocator
	std::optional<vk::AllocationCallbacks> allocation_callbacks = std::nullopt;
};

class Device : public vk::raii::Device {
public:
	explicit Device(std::nullptr_t) : vk::raii::Device{nullptr}, allocator{VK_NULL_HANDLE} {}

	Device(VmaAllocator allocator, vk::raii::Device&& device)
			: vk::raii::Device{std::move(device)}, allocator(allocator) {}

	~Device() {
		if (allocator != VK_NULL_HANDLE) {
			vmaDestroyAllocator(allocator);
		}
	}

	void swap(Device& other) noexcept {
		std::swap(allocator, other.allocator);
		std::swap(static_cast<vk::raii::Device&>(*this), static_cast<vk::raii::Device&>(other));
	}

	Device(const Device& other) = delete;
	Device& operator=(const Device& other) = delete;

	Device(Device&& other) noexcept : vk::raii::Device{nullptr}, allocator{VK_NULL_HANDLE} {
		swap(other);
	}

	Device& operator=(Device&& other) noexcept {
		swap(other);
		return *this;
	}

	using vk::raii::Device::createCommandPool;

private:
	VmaAllocator allocator = VK_NULL_HANDLE;
};

class CommandPoolBuilder {
public:
	CommandPoolBuilder(Device& device) : device{device} {}

	CommandPoolBuilder& with_queue_family_index(size_t index) {
		queue_family_index = index;
		return *this;
	}

	CommandPoolBuilder& with_flags(vk::CommandPoolCreateFlagBits required_flags) {
		flags = required_flags;
		return *this;
	}

	vk::raii::CommandPool create() const {
		vk::CommandPoolCreateInfo create_info{
				.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				.queueFamilyIndex = static_cast<uint32_t>(queue_family_index),
		};

		auto command_pool = device.createCommandPool(create_info, nullptr);
		if (not command_pool) {
			throw std::runtime_error{
					std::format("Unable to create a Vulkan Command Pool: {}", vk::to_string(command_pool.error()))};
		}

		return std::move(*command_pool);
	}

private:
	Device& device;
	size_t queue_family_index = 0;
	vk::CommandPoolCreateFlagBits flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
};

class Surface : public vk::raii::SurfaceKHR {
public:
	using vk::raii::SurfaceKHR::CppType;
	using vk::raii::SurfaceKHR::CType;

	using vk::raii::SurfaceKHR::SurfaceKHR;

	CType native_handle() const noexcept {
		return static_cast<CType>(static_cast<CppType>(*this));
	}

	CppType cpp_handle() const noexcept {
		return static_cast<CType>(static_cast<CppType>(*this));
	}
};

class SurfaceBuilder {
public:
	explicit SurfaceBuilder(Instance& instance, vis::Window* window) : instance{instance}, window{window} {}

	Surface build() {
		auto vk_surface = window->create_renderer_surface(instance.native_handle(), nullptr);
		return Surface{instance, vk_surface, nullptr};
	}

private:
	Instance& instance;
	vis::Window* window;
};

class PhysicalDevice {
public:
	PhysicalDevice() : physical_device{nullptr} /*, surface{nullptr}*/ {}

	PhysicalDevice(vk::raii::PhysicalDevice&& device, Surface* surface = nullptr)
			: physical_device{std::move(device)}, surface{surface} {
		properties = physical_device.getProperties2();
		features = physical_device.getFeatures2();
		layers = physical_device.enumerateDeviceLayerProperties();
		extensions = physical_device.enumerateDeviceExtensionProperties();
		queue_families = physical_device.getQueueFamilyProperties2();

		for (const auto& layer : layers) {
			auto layer_extension = physical_device.enumerateDeviceExtensionProperties(std::string{layer.layerName});
			extensions.insert(end(extensions), begin(layer_extension), end(layer_extension));
		}

		configuration["name"] = std::string_view{properties.properties.deviceName};
		configuration["device type"] = vk::to_string(properties.properties.deviceType);
		configuration["api version"] = vk_version_to_string(properties.properties.apiVersion);

#define ENUMERATE_FEATURE(feature)                                                                                     \
	{                                                                                                                    \
		YAML::Node node;                                                                                                   \
		node[#feature] = bool(features.features.feature);                                                                  \
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

		for (const auto& layer : layers) {
			YAML::Node node;
			node["name"] = std::string_view{layer.layerName};
			node["description"] = (std::string_view{layer.description});
			node["spec version"] = vk_version_to_string(layer.specVersion);
			node["implementation version"] = layer.implementationVersion;
			configuration["layers"].push_back(node);
		}

		for (const auto& extension : extensions) {
			configuration["extension"].push_back(std::string_view{extension.extensionName});
		}

		auto queue_family_index = 0u;
		for (const auto& queue_family : queue_families) {
			YAML::Node node;
			node["flags"] = vk::to_string(queue_family.queueFamilyProperties.queueFlags);
			node["count"] = queue_family.queueFamilyProperties.queueCount;

			if (surface != nullptr) {
				auto preset_support = physical_device.getSurfaceSupportKHR(queue_family_index, surface->cpp_handle());
				node["preset support"] = static_cast<bool>(preset_support);
			}

			configuration["queue families"].push_back(node);

			++queue_family_index;
		}
	}

	void swap(PhysicalDevice& other) noexcept {
		std::swap(this->physical_device, other.physical_device);
		std::swap(this->surface, other.surface);
		std::swap(this->properties, other.properties);
		std::swap(this->features, other.features);
		std::swap(this->layers, other.layers);
		std::swap(this->extensions, other.extensions);
		std::swap(this->queue_families, other.queue_families);
		this->configuration = YAML::Clone(other.configuration);
	}

	PhysicalDevice(PhysicalDevice&) = delete;
	PhysicalDevice& operator=(PhysicalDevice&) = delete;

	PhysicalDevice(PhysicalDevice&& other) noexcept : physical_device{nullptr}, surface{nullptr} {
		swap(other);
	}

	PhysicalDevice& operator=(PhysicalDevice&& other) noexcept {
		swap(other);
		return *this;
	}

	std::string_view name() const {
		return std::string_view{properties.properties.deviceName};
	}

	bool has_any(std::span<vk::PhysicalDeviceType> types) const {
		return std::any_of(begin(types), end(types),
											 [this](vk::PhysicalDeviceType type) { return properties.properties.deviceType == type; });
	}

	bool has_not_any(std::span<vk::PhysicalDeviceType> types) const {
		return not std::any_of(begin(types), end(types),
													 [this](vk::PhysicalDeviceType type) { return properties.properties.deviceType == type; });
	}

	bool is_discrete() const {
		return properties.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
	}

	bool is_integrated() const {
		return properties.properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu;
	}

	bool is_cpu() const {
		return properties.properties.deviceType == vk::PhysicalDeviceType::eCpu;
	}

	bool is_virtual() const {
		return properties.properties.deviceType == vk::PhysicalDeviceType::eVirtualGpu;
	}

	bool has_preset() const {
		assert(surface != nullptr && "To check for preset the surface should be set");
		for (auto i = 0u; i < queue_families.size(); i++) {
			auto preset_support = physical_device.getSurfaceSupportKHR(i, *surface);
			if (preset_support != 0)
				return true;
		}

		return false;
	}

	bool has_graphic_queue() const {
		return std::any_of(begin(queue_families), end(queue_families), [](const vk::QueueFamilyProperties2& queue) {
			return (queue.queueFamilyProperties.queueCount > 0) &&
						 (queue.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics);
		});
	}

	bool has_compute_queue() const {
		return std::any_of(begin(queue_families), end(queue_families), [](const vk::QueueFamilyProperties2& queue) {
			return (queue.queueFamilyProperties.queueCount > 0) &&
						 (queue.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute);
		});
		return false;
	}

	bool has_transfer_queue() const {
		return std::any_of(begin(queue_families), end(queue_families), [](const vk::QueueFamilyProperties2& queue) {
			return (queue.queueFamilyProperties.queueCount > 0) &&
						 (queue.queueFamilyProperties.queueFlags & vk::QueueFlagBits::eTransfer);
		});
		return false;
	}

	bool has_extension(std::string_view extension_name) const {
		auto does_extension_math = [extension_name](const vk::ExtensionProperties& ext) {
			return std::string_view{ext.extensionName} == extension_name;
		};

		return std::find_if(begin(extensions), end(extensions), does_extension_math) != end(extensions);
	}

	bool has_extensions(std::span<std::string_view> required_extensions) const {
		return std::all_of(begin(required_extensions), end(required_extensions),
											 [this](std::string_view extension_name) { return has_extension(extension_name); });
	}

	YAML::Node dump() const {
		return configuration;
	}

	Device create_device(Instance& instance) const {
		auto queue_info_vec = std::vector<vk::DeviceQueueCreateInfo>{};
		for (auto i = 0u; i < queue_families.size(); i++) {
			auto priorities = std::vector<float>(queue_families[i].queueFamilyProperties.queueCount, 1.0f);
			queue_info_vec.emplace_back(vk::DeviceQueueCreateInfo{
					.queueFamilyIndex = i,
					.queueCount = queue_families[i].queueFamilyProperties.queueCount,
					.pQueuePriorities = priorities.data(),
			});
		}

		auto enabled_layers = layers | std::views::transform([](const auto& layer) { return layer.layerName; }) |
													std::ranges::to<std::vector<const char*>>();
		auto enabled_extensions = extensions |
															std::views::transform([](const auto& extension) { return extension.extensionName; }) |
															std::ranges::to<std::vector<const char*>>();

		auto device_create_info = vk::DeviceCreateInfo{};
		device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_info_vec.size());
		device_create_info.pQueueCreateInfos = queue_info_vec.data();
		device_create_info.enabledLayerCount = static_cast<uint32_t>(enabled_layers.size());
		device_create_info.ppEnabledLayerNames = enabled_layers.data();
		device_create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_extensions.size());
		device_create_info.ppEnabledExtensionNames = enabled_extensions.data();

		auto device = physical_device.createDevice(device_create_info);
		if (not device) {
			throw std::runtime_error{std::format("Unable to create a Vulkan Device: {}", vk::to_string(device.error()))};
		}

		VmaVulkanFunctions functions = {};
		functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

		auto allocator_info = VmaAllocatorCreateInfo{
				.flags = 0,
				.physicalDevice = static_cast<vk::raii::PhysicalDevice::CType>(
						static_cast<vk::raii::PhysicalDevice::CppType>(physical_device)),
				.device = static_cast<vk::raii::Device::CType>(static_cast<vk::raii::Device::CppType>(*device)),
				.preferredLargeHeapBlockSize = 0, // default
				.pAllocationCallbacks = nullptr,	//
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
	vk::raii::PhysicalDevice physical_device;
	Surface* surface;
	vk::PhysicalDeviceProperties2 properties;
	vk::PhysicalDeviceFeatures2 features;
	std::vector<vk::LayerProperties> layers;
	std::vector<vk::ExtensionProperties> extensions;
	std::vector<vk::QueueFamilyProperties2> queue_families;

	YAML::Node configuration;
};

class PhysicalDeviceSelector {
public:
	PhysicalDeviceSelector(Instance& intance, Surface* surface = nullptr) : intance{intance}, surface{surface} {}

	std::vector<PhysicalDevice> enumerate_all() const {
		auto devices = intance.enumeratePhysicalDevices();
		if (not devices) {
			return {};
		}
		std::vector<PhysicalDevice> result;
		for (auto&& device : *devices)
			result.emplace_back(PhysicalDevice{std::move(device), surface});

		return result;
	}

	PhysicalDeviceSelector& allow_discrete_device() {
		allowed_physical_device_types.push_back(vk::PhysicalDeviceType::eDiscreteGpu);
		return *this;
	}

	PhysicalDeviceSelector& allow_integrate_device() {
		allowed_physical_device_types.push_back(vk::PhysicalDeviceType::eIntegratedGpu);
		return *this;
	}

	PhysicalDeviceSelector& set_require_preset() {
		require_preset_queue = false;
		return *this;
	}

	PhysicalDeviceSelector& set_require_graphic() {
		require_graphic_queue = false;
		return *this;
	}

	PhysicalDeviceSelector& set_require_compute() {
		require_compute_queue = false;
		return *this;
	}

	PhysicalDeviceSelector& set_require_transfer() {
		require_transfer_queue = false;
		return *this;
	}

	PhysicalDeviceSelector& add_required_extensions(std::span<const char*> extensions) {
		required_gpu_extensions.insert(std::end(required_gpu_extensions), std::begin(extensions), std::end(extensions));
		return *this;
	}

	PhysicalDeviceSelector& with_surface(Surface* desired_surface) {
		surface = desired_surface;
		return *this;
	}

	std::expected<PhysicalDevice, std::string> select() {
		auto devices = intance.enumeratePhysicalDevices();
		if (not devices) {
			return std::unexpected{"no available gpus"};
		}

		std::vector<PhysicalDevice> physical_devices;
		for (auto&& device : *devices)
			physical_devices.emplace_back(PhysicalDevice{std::move(device), surface});

		erase_if(physical_devices, [this](const PhysicalDevice& physical_device) {
			return physical_device.has_not_any(allowed_physical_device_types);
		});

		if (require_preset_queue) {
			erase_if(physical_devices,
							 [](const PhysicalDevice& physical_device) { return not physical_device.has_preset(); });
		}

		if (require_graphic_queue) {
			erase_if(physical_devices,
							 [](const PhysicalDevice& physical_device) { return not physical_device.has_graphic_queue(); });
		}

		if (require_compute_queue) {
			erase_if(physical_devices,
							 [](const PhysicalDevice& physical_device) { return not physical_device.has_compute_queue(); });
		}

		if (require_transfer_queue) {
			erase_if(physical_devices,
							 [](const PhysicalDevice& physical_device) { return not physical_device.has_transfer_queue(); });
		}

		if (physical_devices.empty())
			return std::unexpected("No suitable GPU found");

		if (physical_devices.size() == 1) {
			return std::move(physical_devices.front());
		}

		// more suitable devices
		if (is_discrete_gpu_allowed()) {
			auto discrete_gpu_it = find_first_discrete_gpu(physical_devices);
			if (discrete_gpu_it != end(physical_devices)) {
				return std::move(*discrete_gpu_it);
			}
		}

		if (is_integrated_gpu_allowed()) {
			auto integrated_gpu_it = find_first_integrated_gpu(physical_devices);
			if (integrated_gpu_it != end(physical_devices)) {
				return std::move(*integrated_gpu_it);
			}
		}

		return std::unexpected("No suitable GPU found");
	}

	bool is_discrete_gpu_allowed() const {
		return std::find(begin(allowed_physical_device_types), end(allowed_physical_device_types),
										 vk::PhysicalDeviceType::eDiscreteGpu) != end(allowed_physical_device_types);
	}

	bool is_integrated_gpu_allowed() const {
		return std::find(begin(allowed_physical_device_types), end(allowed_physical_device_types),
										 vk::PhysicalDeviceType::eIntegratedGpu) != end(allowed_physical_device_types);
	}

	auto find_first_discrete_gpu(const std::vector<PhysicalDevice>& physical_devices) const noexcept
			-> decltype(begin(physical_devices)) {
		return std::find_if(begin(physical_devices), end(physical_devices),
												[](const PhysicalDevice& device) { return device.is_discrete(); });
	}

	auto find_first_integrated_gpu(const std::vector<PhysicalDevice>& physical_devices) const noexcept
			-> decltype(begin(physical_devices)) {
		return std::find_if(begin(physical_devices), end(physical_devices),
												[](const PhysicalDevice& device) { return device.is_integrated(); });
	}

	auto find_first_discrete_gpu(std::vector<PhysicalDevice>& physical_devices) noexcept
			-> decltype(begin(physical_devices)) {
		return std::find_if(begin(physical_devices), end(physical_devices),
												[](const PhysicalDevice& device) { return device.is_discrete(); });
	}

	auto find_first_integrated_gpu(std::vector<PhysicalDevice>& physical_devices) noexcept
			-> decltype(begin(physical_devices)) {
		return std::find_if(begin(physical_devices), end(physical_devices),
												[](const PhysicalDevice& device) { return device.is_integrated(); });
	}

private:
	Instance& intance;
	std::vector<const char*> required_gpu_extensions;
	Surface* surface{nullptr};

	std::vector<vk::PhysicalDeviceType> allowed_physical_device_types;

	bool require_preset_queue{true};
	bool require_graphic_queue{true};
	bool require_compute_queue{true};
	bool require_transfer_queue{true};
};

class Texture {
public:
	bool is_stencil() const {
		return true;
	}
	bool is_depth() const {
		return true;
	}
	bool is_color() const {
		return true;
	}

	vk::Format format() const {
		return vk::Format{};
	}

	vk::ImageLayout layout() const {
		return vk::ImageLayout{};
	}
};

class RenderPassBuilder {
public:
	explicit RenderPassBuilder(vk::raii::Device& device) : device{device} {}

	RenderPassBuilder& add_attachment(std::shared_ptr<Texture> attachment) {
		attachments.push_back(attachment);
		return *this;
	}

	RenderPassBuilder& add_attachments(std::span<std::shared_ptr<Texture>> required_attachments) {
		attachments.insert(end(attachments), begin(required_attachments), end(required_attachments));
		return *this;
	}

	RenderPassBuilder& add_load_op(vk::AttachmentLoadOp load_op) {
		load_ops.push_back(load_op);
		return *this;
	}

	RenderPassBuilder& add_store_op(vk::AttachmentStoreOp store_op) {
		store_ops.push_back(store_op);
		return *this;
	}

	RenderPassBuilder& add_load_ops(std::span<vk::AttachmentLoadOp> required_load_ops) {
		load_ops.insert(end(load_ops), begin(required_load_ops), end(required_load_ops));
		return *this;
	}

	RenderPassBuilder& add_store_ops(std::span<vk::AttachmentStoreOp> required_store_op) {
		store_ops.insert(end(store_ops), begin(required_store_op), end(required_store_op));
		return *this;
	}

	vk::raii::RenderPass build() {
		std::vector<vk::AttachmentDescription2> attachment_descriptions;
		std::vector<vk::AttachmentReference2> color_attachment_references;
		std::vector<vk::AttachmentReference2> depth_stencil_attachment_references;

		for (auto i = 0u; i < attachments.size(); i++) {
			auto& attachment = attachments[i];
			auto store_op = store_ops[i];
			auto load_op = load_ops[i];
			auto layout = image_layouts[i];

			attachment_descriptions.emplace_back(vk::AttachmentDescription2{
					.format = attachment->format(),
					.samples = vk::SampleCountFlagBits::e1,
					.loadOp = attachment->is_stencil() ? vk::AttachmentLoadOp::eDontCare : load_op,
					.storeOp = attachment->is_stencil() ? vk::AttachmentStoreOp::eDontCare : store_op,
					.stencilLoadOp = attachment->is_stencil() ? load_op : vk::AttachmentLoadOp::eDontCare,
					.stencilStoreOp = attachment->is_stencil() ? store_op : vk::AttachmentStoreOp::eDontCare,
					.initialLayout = attachment->layout(),
					.finalLayout = layout,
			});

			if (attachment->is_color()) {
				color_attachment_references.emplace_back(vk::AttachmentReference2{
						.attachment = i,
						.layout = vk::ImageLayout::eColorAttachmentOptimal,
				});
			} else {
				color_attachment_references.emplace_back(vk::AttachmentReference2{
						.attachment = i,
						.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
				});
			}
		}

		vk::SubpassDescription2 subpass_description{
				.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
				.colorAttachmentCount = static_cast<uint32_t>(color_attachment_references.size()),
				.pColorAttachments = color_attachment_references.data(),
				.pDepthStencilAttachment = nullptr,
		};

		vk::SubpassDependency2 subpass_dependency{
				.srcSubpass = vk::SubpassExternal,
				// .dstSubpass = 0,
				.srcStageMask =
						vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
				.dstStageMask =
						vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
				.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
		};

		auto render_pass_info = vk::RenderPassCreateInfo2{
				.attachmentCount = static_cast<uint32_t>(attachment_descriptions.size()),
				.pAttachments = attachment_descriptions.data(),
				.subpassCount = 1,
				.pSubpasses = &subpass_description,
				.dependencyCount = 1,
				.pDependencies = &subpass_dependency,
		};
		auto render_pass = device.createRenderPass2(render_pass_info);
		if (not render_pass) {
			throw std::runtime_error{
					std::format("Unable to create a Vulkan RenderPass: {}", vk::to_string(render_pass.error()))};
		}
		return vk::raii::RenderPass{std::move(*render_pass)};
	}

private:
	vk::raii::Device& device;
	std::vector<std::shared_ptr<Texture>> attachments;
	std::vector<vk::ImageLayout> image_layouts;
	std::vector<vk::AttachmentLoadOp> load_ops;
	std::vector<vk::AttachmentStoreOp> store_ops;
};

} // namespace vkh
