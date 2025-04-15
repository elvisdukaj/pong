module;

#include <vulkan/vulkan_raii.hpp>
#include <yaml-cpp/yaml.h>

#include <cassert>

export module vulkan_helper;

import std;

// namespace views = std::ranges::views;

namespace {

// int score_gpu_type(const vk::raii::PhysicalDevice& physical_device) {
// 	int score = 0;
// 	auto props = physical_device.getProperties();
// 	score += props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu ? 4000 : 0;
// 	score += props.deviceType == vk::PhysicalDeviceType::eIntegratedGpu ? 500 : 0;
// 	score += props.deviceType == vk::PhysicalDeviceType::eCpu ? 100 : 0;
// 	score += props.deviceType == vk::PhysicalDeviceType::eVirtualGpu ? 50 : 0;
// 	score += props.deviceType == vk::PhysicalDeviceType::eOther ? 10 : 0;
// 	return score;
// }

// int score_features(const vk::PhysicalDevice& physical_device) {
// 	int score = 0;
// 	auto features = physical_device.getFeatures();
// 	score += features.geometryShader ? 100 : 0;
// 	score += features.tessellationShader ? 200 : 0;
// 	return score;
// }

// int score_queue_family(const vk::PhysicalDevice& physical_device) {
// 	int score = 0;
// 	auto queue_family_props = physical_device.getQueueFamilyProperties();
// 	for (const auto& prop : queue_family_props) {
// 		score += prop.queueFlags & vk::QueueFlagBits::eGraphics ? 500 : 0;
// 		score += prop.queueFlags & vk::QueueFlagBits::eCompute ? 200 : 0;
// 	}
// 	return score;
// }

// int score_gpu_vulkan_version(const vk::raii::PhysicalDevice& physical_device) {
// 	int score = 0;
// 	vk::PhysicalDeviceProperties props = physical_device.getProperties();
// 	score += props.apiVersion == VK_API_VERSION_1_4 ? 100 : 0;
// 	score += props.apiVersion == VK_API_VERSION_1_3 ? 90 : 0;
// 	score += props.apiVersion == VK_API_VERSION_1_2 ? 80 : 0;
// 	return score;
// }

// int score_gpu(const vk::raii::PhysicalDevice& physical_device) {
// 	int score = 0;

// 	score += score_gpu_type(physical_device);
// 	score += score_features(physical_device);
// 	score += score_queue_family(physical_device);
// 	score += score_gpu_vulkan_version(physical_device);
// 	return score;
// }

// // struct ScoredGPU {
// // 	vk::raii::PhysicalDevice device;
// // 	int score;
// // };

// // // std::vector<ScoredGPU> enumerated_scored_gpus(vk::raii::Instance& vk_instance) {
// // // 	return vk_instance.enumeratePhysicalDevices()
// // // 			.and_then([](const std::vector<vk::raii::PhysicalDevice>& devices)
// // // 										-> std::expected<std::vector<ScoredGPU>, vk::Result> {
// // // 				auto scores = devices | std::views::transform(score_gpu);

// // // 				auto map_gpu_score = [](std::tuple<const vk::raii::PhysicalDevice&, int> g) {
// // // 					return ScoredGPU{.device = std::get<0>(g), .score = std::get<1>(g)};
// // // 				};

// // // 				// clang-format off
// // // 				return std::ranges::views::zip(devices, scores)
// // // 					| std::views::transform(map_gpu_score)
// // // 					| std::ranges::to<std::vector<ScoredGPU>>();
// // // 				// clang-format on
// // // 			})
// // // 			.value_or(std::vector<ScoredGPU>{});
// // // }

} // namespace

export namespace vk::raii {
using vk::raii::Instance;
}

export namespace vkh {

// constexpr std::vector<const char*> get_physical_device_extensions() {
// 	std::vector<const char*> required_extensions = {};

// #if defined(__APPLE__)
// 	required_extensions.push_back("VK_KHR_portability_subset");
// #endif
// 	return required_extensions;
// }

std::string vk_version_to_string(uint32_t version) {
	return std::format("{}.{}.{}", VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));
}

std::vector<const char*> get_required_extensions() {
	std::vector<const char*> required_extensions = {VK_KHR_SURFACE_EXTENSION_NAME};

#if defined(__APPLE__)
	required_extensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
	required_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
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

class VulkanContext;
class VulkanInstanceBuilder;

class VulkanContext {
	friend VulkanInstanceBuilder;

public:
	VulkanContext() {
		api_version = context.enumerateInstanceVersion();
		config_["api_version"] = vk_version_to_string(api_version);

		avilable_extensions = context.enumerateInstanceExtensionProperties();
		avilable_layers = context.enumerateInstanceLayerProperties();

		for (const auto& layer : avilable_layers) {
			YAML::Node layer_conf;
			layer_conf["name"] = std::string_view{layer.layerName};
			layer_conf["description"] = std::string_view{layer.description};
			layer_conf["implementation version"] = layer.implementationVersion;
			layer_conf["spec version"] = layer.specVersion;
			config_["available layers"].push_back(layer_conf);

			auto layer_extensions = context.enumerateInstanceExtensionProperties(std::string{layer.layerName});
			avilable_extensions.insert(end(avilable_extensions), begin(layer_extensions), end(layer_extensions));
		}

		for (const auto& extension : avilable_extensions) {
			YAML::Node ext_conf;
			ext_conf["name"] = std::string_view{extension.extensionName};
			ext_conf["spec version"] = extension.specVersion;
			config_["available extensions"].push_back(ext_conf);
		}
	}

	uint32_t api_version;

	std::vector<const char*> get_avilable_layers() const {
		auto to_layer_name = [](const vk::LayerProperties& layer) -> const char* { return layer.layerName; };

		return std::ranges::views::transform(avilable_layers, to_layer_name) | std::ranges::to<std::vector<const char*>>();
	}

	std::vector<const char*> get_avilable_extensions() const {
		auto to_extension_name = [](const vk::ExtensionProperties& extension) -> const char* {
			return extension.extensionName;
		};

		return std::ranges::views::transform(avilable_extensions, to_extension_name) |
					 std::ranges::to<std::vector<const char*>>();
	}

	bool has_extension(std::string_view extension_name) const {
		auto match_extension_name = [extension_name](const vk::ExtensionProperties& extension) -> bool {
			return std::string_view{extension.extensionName} == extension_name;
		};

		auto it = std::find_if(begin(avilable_extensions), end(avilable_extensions), match_extension_name);
		return it != end(avilable_extensions);
	}

	bool has_extensions(std::span<const char*> extensions) {
		return std::all_of(begin(extensions), end(extensions),
											 [this](const char* extension_name) { return has_extension(extension_name); });
	}

	bool has_layer(std::string_view layer_name) const {
		auto match_layer_name = [layer_name](const vk::LayerProperties& layer) -> bool {
			return std::string_view{layer.layerName} == layer_name;
		};

		auto it = std::find_if(begin(avilable_layers), end(avilable_layers), match_layer_name);
		return it != end(avilable_layers);
	}

	bool has_layers(std::span<const char*> layers) {
		return std::all_of(begin(layers), end(layers), [this](const char* layer_name) { return has_layer(layer_name); });
	}

	YAML::Node serialize() const {
		return config_;
	}

private:
	vk::raii::Context context;

	std::vector<vk::ExtensionProperties> avilable_extensions;
	std::vector<vk::LayerProperties> avilable_layers;
	YAML::Node config_;
};

class VulkanInstanceBuilder {
public:
	VulkanInstanceBuilder(VulkanContext& context) : context{context} {}

	VulkanInstanceBuilder& with_minimum_required_instance_version(uint32_t version) {
		minimim_instance_version = version;
		return *this;
	}

	VulkanInstanceBuilder& with_app_name(std::string_view name) {
		app_name = name;
		return *this;
	}

	VulkanInstanceBuilder& with_app_version(uint32_t version) {
		app_version = version;
		return *this;
	}

	VulkanInstanceBuilder& with_app_version(int major, int minor, int patch) {
		app_version = VK_MAKE_VERSION(major, minor, patch);
		return *this;
	}

	VulkanInstanceBuilder& with_engine_name(std::string_view name) {
		engine_name = name;
		return *this;
	}

	VulkanInstanceBuilder& with_engine_version(uint32_t version) {
		engine_version = version;
		return *this;
	}

	VulkanInstanceBuilder& with_engine_version(int major, int minor, int patch) {
		engine_version = VK_MAKE_VERSION(major, minor, patch);
		return *this;
	}

	VulkanInstanceBuilder& add_required_layer(const char* layer_name) {
		required_layers.push_back(layer_name);
		return *this;
	}

	VulkanInstanceBuilder& add_required_layers(std::span<const char*> layers) {
		required_layers.insert(required_layers.end(), begin(layers), end(layers));
		return *this;
	}

	VulkanInstanceBuilder& add_required_extension(const char* extension) {
		required_extensions.push_back(extension);
		return *this;
	}

	VulkanInstanceBuilder& add_required_extensions(std::vector<const char*> extensions) {
		required_extensions.insert(required_extensions.end(), begin(extensions), end(extensions));
		return *this;
	}

	VulkanInstanceBuilder& with_app_flags(vk::InstanceCreateFlags instance_flag) {
		flags = instance_flag;
		return *this;
	}

	vk::raii::Instance build() {
		if (required_api_version > context.api_version) {
			throw std::runtime_error{std::format("The minimum vulkan version is {} bu the minimum required is {}",
																					 vk_version_to_string(context.api_version),
																					 vk_version_to_string(minimim_instance_version))};
		}

		auto avilable_layers = context.get_avilable_layers();
		if (not context.has_layers(required_layers)) {
			throw std::runtime_error("Some required layers are not avilable");
		}

		auto avilable_extensions = context.get_avilable_extensions();
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

		return std::move(*instance);
	}

private:
	VulkanContext& context;

	uint32_t minimim_instance_version = VK_MAKE_VERSION(1, 0, 0);
	uint32_t required_api_version = VK_API_VERSION_1_0;

	// VkApplicationInfo
	std::string app_name = "";
	std::string engine_name = "vis game engine";
	uint32_t app_version = 0;
	uint32_t engine_version = VK_MAKE_API_VERSION(0, 0, 0, 1);

	// VLInstanceCreateInfo
	std::vector<const char*> required_layers;
	std::vector<const char*> required_extensions;
	vk::InstanceCreateFlags flags;
	std::vector<vk::BaseOutStructure> nexts;

	// Custom allocator
	std::optional<vk::AllocationCallbacks> allocation_callbacks = std::nullopt;

	// bool use_debug_messenger = false;
	// bool with_yaml_serialization = true;
};

} // namespace vkh
