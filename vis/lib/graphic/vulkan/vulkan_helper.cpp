module;

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <yaml-cpp/yaml.h>

#include <cassert>

export module vkh;

export import std;
import vulkan_hpp;
import vis.window;

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

using vk::ColorSpaceKHR;
using vk::Format;
using vk::ImageLayout;
using vk::ImageUsageFlagBits;
using vk::PresentModeKHR;

constexpr std::vector<const char*> get_physical_device_extensions() noexcept {
	std::vector<const char*> required_extensions = {
			vk::KHRSwapchainExtensionName,
			vk::KHRDynamicRenderingExtensionName,
			vk::KHRFormatFeatureFlags2ExtensionName,
	};

#if defined(__APPLE__)
	required_extensions.push_back(vk::PhysicalDevicePortabilitySubsetFeaturesKHR);
#endif
	return required_extensions;
}

std::string vk_version_to_string(uint32_t version) noexcept {
	return std::format("{}.{}.{}", vk::apiVersionMajor(version), vk::apiVersionMinor(version),
										 vk::apiVersionPatch(version));
}

std::vector<const char*> get_required_extensions() noexcept {
	std::vector<const char*> required_extensions{
			vk::KHRGetPhysicalDeviceProperties2ExtensionName,
			vk::KHRGetSurfaceCapabilities2ExtensionName,
	};

#if defined(__APPLE__)
	required_extensions.push_back(vk::EXTMetalSurfaceExtensionName);
	required_extensions.push_back(vk::KHRPortabilityEnumerationExtensionName);
#endif

	return required_extensions;
}

std::vector<const char*> get_required_layers() noexcept {
	std::vector<const char*> required_layers;

#if not defined(NDEBUG)
	required_layers.push_back("VK_LAYER_KHRONOS_validation");

#if not defined(__linux)
	required_layers.push_back("VK_LAYER_LUNARG_api_dump");
#endif

#endif

	return required_layers;
}

constexpr ::vk::InstanceCreateFlags get_required_instance_flags() noexcept {
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

	[[nodiscard]] std::vector<const char*> get_available_layers() const noexcept {
		// clang-format off
		return available_layers
			| std::views::transform([](const auto& layer) -> const char* { return layer.layerName; })
			| std::ranges::to<std::vector<const char*>>();
		// clang-format on
	};

	[[nodiscard]] std::vector<const char*> get_available_extensions() const noexcept {
		// clang-format off
		return available_extensions
			| std::views::transform([](const auto& extension) -> const char* { return extension.extensionName; })
			| std::ranges::to<std::vector<const char*>>();
		// clang-format on
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
		return std::ranges::find_if(available_layers, [layer_name](const vk::LayerProperties& layer) -> bool {
						 return std::string_view{layer.layerName} == layer_name;
					 }) != end(available_layers);
	}

	[[nodiscard]] bool has_layers(std::span<const char*> layers) const noexcept {
		return std::ranges::all_of(layers, [this](const char* layer_name) { return has_layer(layer_name); });
	}

	YAML::Node serialize() const noexcept {
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

	explicit Instance(std::nullopt_t) : vk::raii::Instance{nullptr}, api_version{} {}

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

	Instance(Instance&& other) noexcept : vk::raii::Instance{nullptr}, api_version{0} {
		swap(other);
	}

	Instance& operator=(Instance&& other) noexcept {
		swap(other);
		return *this;
	}

	[[nodiscard]] uint32_t get_api_version() const noexcept {
		return api_version;
	}

	CppType cpp_type() const noexcept {
		return static_cast<CppType>(*this);
	}

	CType native_handle() const noexcept {
		return static_cast<CType>(cpp_type());
	}

	using vk::raii::Instance::enumeratePhysicalDevices;

private:
	uint32_t api_version;
	YAML::Node config;
};

class InstanceBuilder {
public:
	explicit InstanceBuilder(Context& context) : context{context} {}

	InstanceBuilder& with_minimum_required_instance_version(int variant, int major, int minor, int patch) noexcept {
		minimum_instance_version = vk::makeApiVersion(variant, major, minor, patch);
		return *this;
	}

	InstanceBuilder& with_maximum_required_instance_version(int variant, int major, int minor, int patch) noexcept {
		maximum_instance_version = vk::makeApiVersion(variant, major, minor, patch);
		return *this;
	}

	InstanceBuilder& with_app_name(std::string_view name) noexcept {
		app_name = name;
		return *this;
	}

	InstanceBuilder& with_app_version(uint32_t version) noexcept {
		app_version = version;
		return *this;
	}

	InstanceBuilder& with_app_version(int major, int minor, int patch) noexcept {
		app_version = vk::makeApiVersion(0, major, minor, patch);
		return *this;
	}

	InstanceBuilder& with_engine_name(std::string_view name) noexcept {
		engine_name = name;
		return *this;
	}

	InstanceBuilder& with_engine_version(uint32_t version) noexcept {
		engine_version = version;
		return *this;
	}

	InstanceBuilder& with_engine_version(int major, int minor, int patch) noexcept {
		engine_version = vk::makeApiVersion(0, major, minor, patch);
		return *this;
	}

	InstanceBuilder& add_required_layer(const char* layer_name) noexcept {
		required_layers.push_back(layer_name);
		return *this;
	}

	InstanceBuilder& add_required_layers(std::span<const char*> layers) noexcept {
		required_layers.insert(required_layers.end(), begin(layers), end(layers));
		return *this;
	}

	InstanceBuilder& add_required_extension(const char* extension) noexcept {
		required_extensions.push_back(extension);
		return *this;
	}

	InstanceBuilder& add_required_extensions(std::span<const char*> extensions) noexcept {
		required_extensions.insert(required_extensions.end(), begin(extensions), end(extensions));
		return *this;
	}

	InstanceBuilder& with_app_flags(vk::InstanceCreateFlags instance_flag) noexcept {
		flags = instance_flag;
		return *this;
	}

	Instance build() {
		maximum_instance_version = std::min(maximum_instance_version, context.api_version);
		if (minimum_instance_version > maximum_instance_version) {
			throw std::runtime_error{std::format("The minimum vulkan version is {} bu the minimum required is {}",
																					 vk_version_to_string(maximum_instance_version),
																					 vk_version_to_string(minimum_instance_version))};
		}

		auto required_api_version = std::max(minimum_instance_version, maximum_instance_version);

		auto available_layers = context.get_available_layers();
		if (not context.has_layers(required_layers)) {
			throw std::runtime_error("Some required layers are not available");
		}

		auto available_extensions = context.get_available_extensions();
		if (not context.has_extensions(required_extensions)) {
			throw std::runtime_error("Some required extensions are not available");
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

		return Instance{std::move(*instance), required_api_version};
	}

private:
	Context& context;

	uint32_t minimum_instance_version = vk::makeApiVersion(0, 1, 0, 0);
	uint32_t maximum_instance_version = vk::makeApiVersion(0, 1, 0, 0);

	// VkApplicationInfo
	std::string app_name;
	std::string engine_name = "vis game engine";
	uint32_t app_version = 0;
	uint32_t engine_version = vk::makeApiVersion(0, 0, 0, 1);
	// VLInstanceCreateInfo
	std::vector<const char*> required_layers;
	std::vector<const char*> required_extensions;
	vk::InstanceCreateFlags flags;

	// Custom allocator
	std::optional<vk::AllocationCallbacks> allocation_callbacks = std::nullopt;
};

class CommandPool : public vk::raii::CommandPool {
public:
	using vk::raii::CommandPool::CommandPool;
	using vk::raii::CommandPool::CppType;
	using vk::raii::CommandPool::CType;

	// explicit CommandPool(std::nullptr_t) : vk::raii::CommandPool{nullptr} {}
};

class Device : public vk::raii::Device {
public:
	explicit Device(std::nullptr_t) : vk::raii::Device{nullptr}, allocator{VK_NULL_HANDLE} {}

	Device(VmaAllocator allocator, vk::raii::Device&& device) noexcept
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
	explicit CommandPoolBuilder(Device& device) : device{device} {}

	CommandPoolBuilder& with_queue_family_index(size_t index) {
		queue_family_index = index;
		return *this;
	}

	CommandPoolBuilder& with_flags(vk::CommandPoolCreateFlagBits required_flags) noexcept {
		flags = required_flags;
		return *this;
	}

	[[nodiscard]] CommandPool create() const {
		vk::CommandPoolCreateInfo create_info{
				.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				.queueFamilyIndex = static_cast<uint32_t>(queue_family_index),
		};

		auto command_pool = device.createCommandPool(create_info, nullptr);
		if (not command_pool) {
			throw std::runtime_error{
					std::format("Unable to create a Vulkan Command Pool: {}", vk::to_string(command_pool.error()))};
		}

		return CommandPool{std::move(*command_pool)};
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

	[[nodiscard]] CType native_handle() const noexcept {
		return static_cast<CType>(static_cast<CppType>(*this));
	}
};

class SurfaceBuilder {
public:
	explicit SurfaceBuilder(Instance& instance, vis::Window* window) : instance{instance}, window{window} {}

	[[nodiscard]] Surface build() const noexcept {
		auto vk_surface = window->create_renderer_surface(instance.native_handle(), nullptr);
		return Surface{instance, vk_surface, nullptr};
	}

private:
	Instance& instance;
	vis::Window* window;
};

class PhysicalDevice {
	friend class PhysicalDeviceSelector;

public:
	PhysicalDevice() : physical_device{nullptr}, surface{nullptr} {
		// std::println("I am empty");
	}

	explicit PhysicalDevice(vk::raii::PhysicalDevice&& device, std::vector<const char*> requirested_required_layers,
													std::vector<const char*> requirested_required_extensions, Surface* surface = nullptr)
			: physical_device{std::move(device)},
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
	YAML::Node configuration;
};

class PhysicalDeviceSelector {
public:
	explicit PhysicalDeviceSelector(Instance& instance, Surface* surface = nullptr)
			: instance{instance}, surface{surface} {}

	[[nodiscard]] std::vector<PhysicalDevice> enumerate_all() const {
		auto devices = instance.enumeratePhysicalDevices();
		if (not devices) {
			return {};
		}
		std::vector<PhysicalDevice> result;
		for (auto&& device : *devices)
			result.emplace_back(std::move(device), std::vector<const char*>{}, std::vector<const char*>{}, surface);

		return result;
	}

	PhysicalDeviceSelector& allow_discrete_device() {
		allowed_physical_device_types.emplace_back(vk::PhysicalDeviceType::eDiscreteGpu);
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

	PhysicalDevice select() {
		auto devices = instance.enumeratePhysicalDevices();
		if (not devices) {
			throw std::runtime_error{"no available gpus"};
		}

		std::vector<PhysicalDevice> physical_devices;
		for (auto&& device : *devices)
			physical_devices.emplace_back(std::move(device), required_gpu_layers, required_gpu_extensions, surface);

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
			throw std::runtime_error{"No suitable GPU found"};

		auto selected_device_iter = end(physical_devices);

		// more suitable devices
		if (is_discrete_gpu_allowed()) {
			auto discrete_gpu_it = find_first_discrete_gpu(physical_devices);
			if (discrete_gpu_it != end(physical_devices)) {
				selected_device_iter = discrete_gpu_it;
			}
		}

		if (selected_device_iter == end(physical_devices) and is_integrated_gpu_allowed()) {
			auto integrated_gpu_it = find_first_integrated_gpu(physical_devices);
			if (integrated_gpu_it != end(physical_devices)) {
				selected_device_iter = integrated_gpu_it;
			}
		}

		if (selected_device_iter == end(physical_devices))
			throw std::runtime_error{"No suitable GPU found"};

		return std::move(*selected_device_iter);
	}

	[[nodiscard]] bool is_discrete_gpu_allowed() const noexcept {
		return std::ranges::find(allowed_physical_device_types, vk::PhysicalDeviceType::eDiscreteGpu) !=
					 end(allowed_physical_device_types);
	}

	[[nodiscard]] bool is_integrated_gpu_allowed() const noexcept {
		return std::ranges::find(allowed_physical_device_types, vk::PhysicalDeviceType::eIntegratedGpu) !=
					 end(allowed_physical_device_types);
	}

	static std::vector<PhysicalDevice>::const_iterator
	find_first_discrete_gpu(const std::vector<PhysicalDevice>& physical_devices) noexcept {
		return std::ranges::find_if(physical_devices, [](const PhysicalDevice& device) { return device.is_discrete(); });
	}

	static std::vector<PhysicalDevice>::const_iterator
	find_first_integrated_gpu(const std::vector<PhysicalDevice>& physical_devices) noexcept {
		return std::ranges::find_if(physical_devices, [](const PhysicalDevice& device) { return device.is_integrated(); });
	}

	static std::vector<PhysicalDevice>::iterator
	find_first_discrete_gpu(std::vector<PhysicalDevice>& physical_devices) noexcept {
		return std::ranges::find_if(physical_devices, [](const PhysicalDevice& device) { return device.is_discrete(); });
	}

	static std::vector<PhysicalDevice>::iterator
	find_first_integrated_gpu(std::vector<PhysicalDevice>& physical_devices) noexcept {
		return std::ranges::find_if(physical_devices, [](const PhysicalDevice& device) { return device.is_integrated(); });
	}

private:
	Instance& instance;
	std::vector<const char*> required_gpu_extensions;
	std::vector<const char*> required_gpu_layers;
	Surface* surface{nullptr};

	std::vector<vk::PhysicalDeviceType> allowed_physical_device_types;

	bool require_preset_queue{true};
	bool require_graphic_queue{true};
	bool require_compute_queue{true};
	bool require_transfer_queue{true};
};

class Texture {
public:
	bool is_stencil() const noexcept {
		return true;
	}
	bool is_depth() const noexcept {
		return true;
	}
	bool is_color() const noexcept {
		return true;
	}

	vk::Format format() const noexcept {
		return vk::Format{};
	}

	vk::ImageLayout layout() const noexcept {
		return vk::ImageLayout{};
	}
};

class RenderPass : public vk::raii::RenderPass {
public:
	using vk::raii::RenderPass::RenderPass;
};

class RenderPassBuilder {
public:
	explicit RenderPassBuilder(vk::raii::Device& device) : device{device} {}

	RenderPassBuilder& add_attachment(std::shared_ptr<Texture>& attachment) {
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

class Image : public vk::Image {
public:
	using vk::Image::Image;

	explicit Image(const vk::Image& image) : vk::Image{image} {}
};

class ImageView : public vk::raii::ImageView {
public:
	using vk::raii::ImageView::ImageView;
};

class SwapChain : public vk::raii::SwapchainKHR {
public:
	using vk::raii::SwapchainKHR::CppType;
	using vk::raii::SwapchainKHR::CType;
	using vk::raii::SwapchainKHR::SwapchainKHR;

	explicit SwapChain(std::nullptr_t) : vk::raii::SwapchainKHR{nullptr} {}

	explicit SwapChain(vk::raii::SwapchainKHR&& swapchain /*, vk::raii::Device& device*/) noexcept
			: vk::raii::SwapchainKHR{std::move(swapchain)} /*, device{&device}*/ {}

	[[nodiscard]] CType native_handle() const noexcept {
		return static_cast<CType>(static_cast<CppType>(*this));
	}

	// vk::raii::Image& get_image(uint32_t index) const noexcept {}
	std::vector<vk::Image> get_images() const noexcept {
		// // clang-format off
		// vk::raii::SwapchainKHR::getImages()
		// 	| std::views::transform([](const ::vk::Image& image) -> ::vkh::Image {
		// 	return ::vkh::Image{image};
		// }) | std::ranges::to<std::vector<::vkh::Image>>;
		// // clang-format on
		return vk::raii::SwapchainKHR::getImages();
	}

	// std::vector<vk::ImageView> get_image_views() const noexcept {}

	// private:
	// 	vk::raii::Device* device{nullptr};
};

class SwapChainBuilder {
public:
	SwapChainBuilder(Surface& surface, Device& device) : surface{surface}, device{device} {}

	SwapChainBuilder& set_flags(vk::SwapchainCreateFlagsKHR required_flags) {
		flags = required_flags;
		return *this;
	}

	SwapChainBuilder& set_min_image_count(uint32_t required_min_count) {
		min_image_count = required_min_count;
		return *this;
	}

	SwapChainBuilder& set_required_format(vk::Format required_format) {
		format = required_format;
		return *this;
	}

	SwapChainBuilder& set_required_color_space(vk::ColorSpaceKHR required_color_space) {
		color_space = required_color_space;
		return *this;
	}

	SwapChainBuilder& set_extent(vk::Extent2D required_extent_2d) {
		extent = required_extent_2d;
		return *this;
	}

	SwapChainBuilder& set_extent(int width, int height) {
		return set_extent({
				.width = static_cast<uint32_t>(width),
				.height = static_cast<uint32_t>(height),
		});
	}

	SwapChainBuilder& set_image_array_layers(uint32_t required_image_array_layers) {
		image_array_layers = required_image_array_layers;
		return *this;
	}

	SwapChainBuilder& set_image_usage(vk::ImageUsageFlags required_image_usage) {
		image_usage = required_image_usage;
		return *this;
	}

	SwapChainBuilder& set_image_sharing_mode(vk::SharingMode required_image_sharing_mode) {
		sharing_mode = required_image_sharing_mode;
		return *this;
	}

	SwapChainBuilder& add_queue_family_index(uint32_t queue_family_index) {
		queue_family_indices.push_back(queue_family_index);
		return *this;
	}

	SwapChainBuilder& add_queue_family_index(std::span<uint32_t> required_queue_family_indexes) {
		queue_family_indices.insert(end(queue_family_indices), begin(required_queue_family_indexes),
																end(required_queue_family_indexes));
		return *this;
	}

	SwapChainBuilder& set_pre_transform(vk::SurfaceTransformFlagBitsKHR required_pre_transform) {
		pre_transform = required_pre_transform;
		return *this;
	}

	SwapChainBuilder& set_composite_alpha(vk::CompositeAlphaFlagBitsKHR required_composite_alpha) {
		composite_alpha = required_composite_alpha;
		return *this;
	}

	SwapChainBuilder& set_present_mode(vk::PresentModeKHR required_present_mode) {
		present_mode = required_present_mode;
		return *this;
	}

	SwapChainBuilder& set_clipped(bool required_clipped) {
		clipped = required_clipped;
		return *this;
	}

	SwapChainBuilder& set_old_swapchain(const SwapChain& use_old_swapchain) {
		old_swapchain = use_old_swapchain.native_handle();
		return *this;
	}

	[[nodiscard]] SwapChain build() const {
		vk::SwapchainCreateInfoKHR swapchain_create_info{
				.flags = flags,
				.surface = surface.native_handle(),
				.minImageCount = min_image_count,
				.imageFormat = format,
				.imageColorSpace = color_space,
				.imageExtent = extent,
				.imageArrayLayers = image_array_layers,
				.imageUsage = image_usage,
				.imageSharingMode = sharing_mode,
				.queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices.size()),
				.pQueueFamilyIndices = queue_family_indices.data(),
				.preTransform = pre_transform,
				.compositeAlpha = composite_alpha,
				.presentMode = present_mode,
				.clipped = clipped,
				.oldSwapchain = VK_NULL_HANDLE,
		};

		return SwapChain{std::move(*device.createSwapchainKHR(swapchain_create_info)) /*, device*/};
	}

private:
	Surface& surface;
	Device& device;
	vk::SwapchainCreateFlagsKHR flags{};
	uint32_t min_image_count = 2;
	vk::Format format = vk::Format::eB8G8R8A8Unorm;
	vk::ColorSpaceKHR color_space = vk::ColorSpaceKHR::eSrgbNonlinear;
	vk::Extent2D extent;
	uint32_t image_array_layers = 1;
	vk::ImageUsageFlags image_usage = vk::ImageUsageFlagBits::eColorAttachment;
	vk::SharingMode sharing_mode = vk::SharingMode::eExclusive;
	std::vector<uint32_t> queue_family_indices;
	vk::SurfaceTransformFlagBitsKHR pre_transform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	vk::CompositeAlphaFlagBitsKHR composite_alpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;
	bool clipped = VK_TRUE;
	vk::SwapchainKHR old_swapchain = VK_NULL_HANDLE;
};

} // namespace vkh
