module;

#include <SDL3/SDL.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vulkan/vulkan_to_string.hpp>
#include <yaml-cpp/yaml.h>

#include <cassert>

export module vis:vulkan;

import std;

import :math;
import :window;

namespace views = std::ranges::views;

namespace {

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

std::string vk_version_to_string(uint32_t version) {
	return std::format("{}.{}.{}", VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));
}

int score_gpu_type(const vk::raii::PhysicalDevice& physical_device) {
	int score = 0;
	auto props = physical_device.getProperties();
	score += props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu ? 4000 : 0;
	score += props.deviceType == vk::PhysicalDeviceType::eIntegratedGpu ? 500 : 0;
	score += props.deviceType == vk::PhysicalDeviceType::eCpu ? 100 : 0;
	score += props.deviceType == vk::PhysicalDeviceType::eVirtualGpu ? 50 : 0;
	score += props.deviceType == vk::PhysicalDeviceType::eOther ? 10 : 0;
	return score;
}

int score_features(const vk::PhysicalDevice& physical_device) {
	int score = 0;
	auto features = physical_device.getFeatures();
	score += features.geometryShader ? 100 : 0;
	score += features.tessellationShader ? 200 : 0;
	return score;
}

int score_queue_family(const vk::PhysicalDevice& physical_device) {
	int score = 0;
	auto queue_family_props = physical_device.getQueueFamilyProperties();
	for (const auto& prop : queue_family_props) {
		score += prop.queueFlags & vk::QueueFlagBits::eGraphics ? 500 : 0;
		score += prop.queueFlags & vk::QueueFlagBits::eCompute ? 200 : 0;
	}
	return score;
}

int score_gpu_vulkan_version(const vk::raii::PhysicalDevice& physical_device) {
	int score = 0;
	vk::PhysicalDeviceProperties props = physical_device.getProperties();
	score += props.apiVersion == VK_API_VERSION_1_4 ? 100 : 0;
	score += props.apiVersion == VK_API_VERSION_1_3 ? 90 : 0;
	score += props.apiVersion == VK_API_VERSION_1_2 ? 80 : 0;
	return score;
}

int score_gpu(const vk::raii::PhysicalDevice& physical_device) {
	int score = 0;

	score += score_gpu_type(physical_device);
	score += score_features(physical_device);
	score += score_queue_family(physical_device);
	score += score_gpu_vulkan_version(physical_device);
	return score;
}

struct ScoredGPU {
	vk::raii::PhysicalDevice device;
	int score;
};

std::vector<ScoredGPU> enumerated_scored_gpus(vk::raii::Instance& vk_instance) {
	return vk_instance.enumeratePhysicalDevices()
			.and_then([](const std::vector<vk::raii::PhysicalDevice>& devices)
										-> std::expected<std::vector<ScoredGPU>, vk::Result> {
				auto scores = devices | std::views::transform(score_gpu);

				auto map_gpu_score = [](std::tuple<const vk::raii::PhysicalDevice&, int> g) {
					return ScoredGPU{.device = std::get<0>(g), .score = std::get<1>(g)};
				};

				// clang-format off
				return std::ranges::views::zip(devices, scores)
					| std::views::transform(map_gpu_score)
					| std::ranges::to<std::vector<ScoredGPU>>();
				// // clang-format on
			})
			.value_or(std::vector<ScoredGPU>{});
}

} // namespace

export namespace vis::vulkan {

class Renderer {
public:
	static std::expected<Renderer, std::string> create(Window* window) {
		try {
			return Renderer{window};
		} catch (const std::exception& exc) {
			return std::unexpected{exc.what()};
		} catch (...) {
			return std::unexpected{"Unable to create a Vulkan instance"};
		}
	}

	~Renderer() = default;

	Renderer(Renderer&) = delete;
	Renderer& operator=(Renderer&) = delete;

	friend void swap(Renderer& lhs, Renderer& rhs) {
		std::swap(lhs.required_extensions, rhs.required_extensions);
		std::swap(lhs.required_layers, rhs.required_layers);
		std::swap(lhs.window, rhs.window);
		std::swap(lhs.context, rhs.context);
		std::swap(lhs.vk_instance, rhs.vk_instance);
		std::swap(lhs.physical_devices, rhs.physical_devices);
		std::swap(lhs.physical_device, rhs.physical_device);
		// std::swap(lhs.gpu_score, rhs.gpu_score);
		// std::swap(lhs.gpu_queue_index, rhs.gpu_queue_index);
		// std::swap(lhs.device, rhs.device);
	}

	Renderer(Renderer&& other) = default;
	Renderer& operator=(Renderer&& rhs) {
		swap(*this, rhs);
		return *this;
	}

	void set_clear_color([[maybe_unused]] const vec4& color) {}
	void clear() {}
	void render() const {
		// SDL_GL_SwapWindow(*context.window);
	}

	void set_viewport([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int width,
										[[maybe_unused]] int height) {}

	std::string show_info() const {
		std::stringstream ss;
		ss << vk_config;
		return ss.str();
	}

private:
	explicit Renderer(Window* window) : window{window}, vk_instance{nullptr}, physical_device{nullptr} {
		create_instance();
		enumerate_gpus();
	}

	void create_instance() {
		auto flags = get_required_instance_flags();
		required_extensions = get_required_extensions();
		required_layers = get_required_layers();

		auto api_version = context.enumerateInstanceVersion();
		vk_config["version"] = vk_version_to_string(api_version);

		{
			auto extensions = context.enumerateInstanceExtensionProperties();
			auto extensions_str = std::vector<std::string>(extensions.size());
			std::transform(begin(extensions), end(extensions), begin(extensions_str),
										 [](auto& extension) { return std::string{extension.extensionName}; });
			vk_config["extensions"] = extensions_str;
		}

		auto layers = context.enumerateInstanceLayerProperties();

		for (const auto& layer : layers) {
			YAML::Node layer_conf;
			layer_conf["name"] = std::string{layer.layerName};
			layer_conf["version"] = vk_version_to_string(layer.specVersion);
			layer_conf["description"] = std::string{layer.description};

			auto extensions = context.enumerateInstanceExtensionProperties(std::string{layer.layerName});
			auto extensions_str = std::vector<std::string>(extensions.size());
			std::transform(begin(extensions), end(extensions), begin(extensions_str),
										 [](auto& extension) { return std::string{extension.extensionName}; });
			layer_conf["extensions"] = extensions_str;
			vk_config["layers"].push_back(layer_conf);
		}

		const auto title = SDL_GetWindowTitle(static_cast<SDL_Window*>(*window));

		vk::ApplicationInfo app_info{
				.pNext = nullptr,
				.pApplicationName = title,
				.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
				.pEngineName = "vis game engine",
				.engineVersion = VK_MAKE_VERSION(1, 0, 1),
				.apiVersion = VK_API_VERSION_1_2,
		};

		vk::InstanceCreateInfo create_info{
				.flags = flags,
				.pApplicationInfo = &app_info,
				.enabledLayerCount = static_cast<uint32_t>(size(required_layers)),
				.ppEnabledLayerNames = required_layers.data(),
				.enabledExtensionCount = static_cast<uint32_t>(size(required_extensions)),
				.ppEnabledExtensionNames = required_extensions.data(),
		};

		auto instance = context.createInstance(create_info);
		if (not instance) {
			throw std::runtime_error(vk::to_string((instance.error())));
		}
		vk_instance = std::move(*instance);
	}

	void enumerate_gpus() {
		physical_devices = enumerated_scored_gpus(vk_instance);

		if (empty(physical_devices)) {
			throw std::runtime_error("No GPUs found!");
		}

		auto order_by_score = [](const ScoredGPU& lhs, const ScoredGPU& rhs) { return lhs.score < rhs.score; };
		std::ranges::sort(physical_devices, order_by_score);


		for (const auto& scored_gpu : physical_devices) {
			const auto& [device, score] = scored_gpu;
			auto properties = device.getProperties();

			YAML::Node physical_device_config;
			physical_device_config["name"] = std::string{properties.deviceName};
			physical_device_config["type"] = vk::to_string(properties.deviceType);
			physical_device_config["api version"] = vk_version_to_string(properties.apiVersion);
			physical_device_config["driver version"] = vk_version_to_string(properties.driverVersion);
			physical_device_config["score"] = score;
			vk_config["gpus"].push_back(physical_device_config);
		}

		physical_device = physical_devices.back().device;
		{
			auto selected_device_properties = physical_device.getProperties();
			YAML::Node selected_device_config;
			selected_device_config["name"] = std::string{selected_device_properties.deviceName};
			selected_device_config["type"] = vk::to_string(selected_device_properties.deviceType);
			selected_device_config["api version"] = vk_version_to_string(selected_device_properties.apiVersion);
			selected_device_config["driver version"] = vk_version_to_string(selected_device_properties.driverVersion);
			vk_config["selected gpu"].push_back(selected_device_config);
		}
	}

	void select_gpu() {
		[[maybe_unused]] auto scored_gpus = enumerated_scored_gpus(vk_instance);
	}

private:
	std::vector<const char*> required_extensions;
	std::vector<const char*> required_layers;

	Window* window;
	vk::raii::Context context;
	vk::raii::Instance vk_instance;
	std::vector<ScoredGPU> physical_devices;
	vk::raii::PhysicalDevice physical_device;
	YAML::Node vk_config;
}; // namespace vis::vk

} // namespace vis::vulkan