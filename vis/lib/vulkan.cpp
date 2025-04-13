module;

#include <SDL3/SDL.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
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
	score += props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu ? 2000 : 0;
	score += props.deviceType == vk::PhysicalDeviceType::eIntegratedGpu ? 500 : 0;
	score += props.deviceType == vk::PhysicalDeviceType::eCpu ? 400 : 0;
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
		// std::swap(lhs.devices, rhs.devices);
		// std::swap(lhs.physical_device, rhs.physical_device);
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
	explicit Renderer(Window* window) : window{window}, vk_instance{nullptr} {
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
			layer_conf["description"] = extensions_str;
			vk_config["layers"].push_back(layer_conf);
		}

		const auto title = SDL_GetWindowTitle(static_cast<SDL_Window*>(*window));

		vk::ApplicationInfo app_info{
				title, VK_MAKE_VERSION(1, 0, 0), "vis game engine", VK_MAKE_VERSION(1, 0, 1), VK_API_VERSION_1_2,
		};
		vk::InstanceCreateInfo create_info{flags,
																			 &app_info,
																			 static_cast<uint32_t>(size(required_layers)),
																			 required_layers.data(),
																			 static_cast<uint32_t>(size(required_extensions)),
																			 required_extensions.data()};

		auto instance = context.createInstance(create_info);
		if (not instance) {
			throw std::runtime_error(vk::to_string((instance.error())));
		}
		vk_instance = std::move(*instance);

		auto physical_devices_expec = vk_instance.enumeratePhysicalDevices();
		if (not physical_devices_expec) {
			throw std::runtime_error(vk::to_string(physical_devices_expec.error()));
		}

		physical_devices =
				*vk_instance.enumeratePhysicalDevices() | std::ranges::to<std::vector<vk::raii::PhysicalDevice>>();

		for (const vk::raii::PhysicalDevice& physical_device : physical_devices) {
			auto properties = physical_device.getProperties();

			YAML::Node physical_device_config;
			auto gpu_score = score_gpu(physical_device);

			physical_device_config["name"] = std::string{properties.deviceName};
			physical_device_config["type"] = vk::to_string(properties.deviceType);
			physical_device_config["api version"] = vk_version_to_string(properties.apiVersion);
			physical_device_config["driver version"] = vk_version_to_string(properties.driverVersion);
			physical_device_config["score"] = gpu_score;
			vk_config["gpu"].push_back(physical_device_config);
		}

		// devices = instance.enumerate_physical_devices(required_layers);

		// if (empty(devices))
		// throw std::runtime_error{"no physical device found"};

		// auto gpu_scores = score_gpus(devices);
		// auto it = std::max_element(begin(gpu_scores), end(gpu_scores));

		// if (*it == 0)
		// throw std::runtime_error{"No suitable GPU scores found"};

		// size_t gpu_idx = static_cast<size_t>(std::distance(begin(gpu_scores), it));
		// gpu_score = *it;
		// physical_device = devices.at(gpu_idx);

		// gpu_queue_index = get_graphic_queue_index(physical_device);

		// device = LogicalDevice::create(physical_device, gpu_queue_index, required_extensions);
	}

private:
	std::vector<const char*> required_extensions;
	std::vector<const char*> required_layers;

	Window* window;
	vk::raii::Context context;
	vk::raii::Instance vk_instance;
	std::vector<vk::raii::PhysicalDevice> physical_devices;

	YAML::Node vk_config;
}; // namespace vis::vk

} // namespace vis::vulkan