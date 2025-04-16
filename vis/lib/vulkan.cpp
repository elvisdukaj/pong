module;

#include <SDL3/SDL.h>

// #include <vulkan/vulkan_raii.hpp>
#include <yaml-cpp/yaml.h>

#include <cassert>

export module vis:vulkan;

import std;

import vulkan_helper;

import :math;
import :window;

namespace views = std::ranges::views;

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
		// std::swap(lhs.required_extensions, rhs.required_extensions);
		// std::swap(lhs.required_layers, rhs.required_layers);
		std::swap(lhs.window, rhs.window);
		std::swap(lhs.context, rhs.context);
		std::swap(lhs.vk_instance, rhs.vk_instance);
		// std::swap(lhs.scored_physical_devices, rhs.scored_physical_devices);
		// std::swap(lhs.physical_device, rhs.physical_device);
		// std::swap(lhs.graphic_queue_index, rhs.graphic_queue_index);
		// std::swap(lhs.transfer_queue_index, rhs.transfer_queue_index);
		// std::swap(lhs.device, rhs.device);
		// std::swap(lhs.gpu_score, rhs.gpu_score);
		// std::swap(lhs.gpu_queue_index, rhs.gpu_queue_index);
		// std::swap(lhs.device, rhs.device);
		std::swap(lhs.vk_config, rhs.vk_config);
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
	explicit Renderer(Window* window)
			: window{window} /*, vk_instance{nullptr}, physical_device{nullptr}, device{nullptr}*/ {
		create_instance();
		enumerate_gpus();
		// create_device();
	}

	void create_instance() {
		auto required_flags = vkh::get_required_instance_flags();
		auto required_extensions = vkh::get_required_extensions();
		auto required_layers = vkh::get_required_layers();

		vk_instance = vkh::InstanceBuilder{context}
											.with_app_name("Pong")
											.with_app_version(0, 1, 1)
											.with_engine_name("vis")
											.with_engine_version(0, 1, 1)
											.with_app_flags(required_flags)
											.add_required_layers(required_layers)
											.add_required_extensions(required_extensions)
											.build();

		vk_config["instance"] = context.serialize();
	}

	void enumerate_gpus() {
		auto device_selector = vkh::PhysicalDeviceSelector{vk_instance};

		for (const auto& device : device_selector) {
			vk_config["GPUs"].push_back(device.dump());
		}

		// 	scored_physical_devices = enumerated_scored_gpus(vk_instance);

		// 	if (empty(scored_physical_devices)) {
		// 		throw std::runtime_error("No GPUs found!");
		// 	}

		// 	auto order_by_score = [](const ScoredGPU& lhs, const ScoredGPU& rhs) { return lhs.score < rhs.score; };
		// 	std::ranges::sort(scored_physical_devices, order_by_score);

		// 	for (const auto& scored_gpu : scored_physical_devices) {
		// 		vk_config["gpus"].push_back(serialize_gpu_to_yaml(scored_gpu));
		// 	}

		// 	physical_device = scored_physical_devices.back().device;
		// 	vk_config["selected gpu"] = std::string{physical_device.getProperties().deviceName};

		// 	graphic_queue_index = select_queue_index_for(vk::QueueFlagBits::eGraphics);
		// 	vk_config["selected graphic queue index"] = graphic_queue_index;

		// 	transfer_queue_index = select_queue_index_for(vk::QueueFlagBits::eTransfer);
		// 	vk_config["selected transfer queue index"] = transfer_queue_index;
	}

	// size_t select_queue_index_for(vk::QueueFlagBits flag) const {
	// 	auto queue_properties = physical_device.getQueueFamilyProperties();

	// 	auto has_graphic_bit = [flag](const vk::QueueFamilyProperties& queue) {
	// 		return queue.queueCount != 0 && queue.queueFlags & flag;
	// 	};
	// 	auto it = std::find_if(begin(queue_properties), end(queue_properties), has_graphic_bit);
	// 	if (it == std::end(queue_properties))
	// 		throw std::runtime_error{"No Graphic queue found!"};

	// 	return static_cast<std::size_t>(std::distance(begin(queue_properties), it));
	// }

	// YAML::Node serialize_gpu_to_yaml(const ScoredGPU& scored_gpu) const {
	// 	const auto properties = scored_gpu.device.getProperties();
	// 	const auto queue_properties = scored_gpu.device.getQueueFamilyProperties();
	// 	const auto extensions = scored_gpu.device.enumerateDeviceExtensionProperties();

	// 	YAML::Node physical_device_config;
	// 	physical_device_config["name"] = std::string{properties.deviceName};
	// 	physical_device_config["type"] = vk::to_string(properties.deviceType);
	// 	physical_device_config["api version"] = vk_version_to_string(properties.apiVersion);
	// 	physical_device_config["driver version"] = vk_version_to_string(properties.driverVersion);

	// 	YAML::Node extensions_config;
	// 	auto extensions_str = std::vector<std::string>(extensions.size());
	// 	std::transform(begin(extensions), end(extensions), begin(extensions_str),
	// 								 [](auto& extension) { return std::string{extension.extensionName}; });
	// 	for (const auto& extension : extensions_str) {
	// 		extensions_config.push_back(extension);
	// 	}
	// 	physical_device_config["extensions"] = extensions_config;

	// 	physical_device_config["score"] = scored_gpu.score;

	// 	for (const auto& queue : queue_properties) {
	// 		YAML::Node queue_config;
	// 		queue_config["flags"] = vk::to_string(queue.queueFlags);
	// 		queue_config["count"] = queue.queueCount;
	// 		physical_device_config["queues"].push_back(queue_config);
	// 	}

	// 	return physical_device_config;
	// }

	// void create_device() {
	// 	float prio[1] = {0.0f};
	// 	std::vector<vk::DeviceQueueCreateInfo> queue_create_info = {vk::DeviceQueueCreateInfo{
	// 			.queueFamilyIndex = static_cast<uint32_t>(graphic_queue_index),
	// 			.queueCount = 1,
	// 			.pQueuePriorities = prio,
	// 	}};

	// 	auto extensions = get_physical_device_extensions();
	// 	auto create_info = vk::DeviceCreateInfo{
	// 			// .flags = VK_QUEUE_GRAPHICS_BIT,
	// 			.queueCreateInfoCount = 1,
	// 			.pQueueCreateInfos = queue_create_info.data(),
	// 			.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
	// 			.ppEnabledExtensionNames = extensions.data(),
	// 	};

	// 	auto expected_device = physical_device.createDevice(create_info);
	// 	if (not expected_device) {
	// 		throw std::runtime_error{vk::to_string(expected_device.error())};
	// 	}

	// 	device = std::move(*expected_device);
	// }

private:
	Window* window;
	vkh::Context context;
	vkh::Instance vk_instance{nullptr};
	// std::vector<ScoredGPU> scored_physical_devices;
	// vk::raii::PhysicalDevice physical_device;
	// size_t graphic_queue_index;
	// size_t transfer_queue_index;
	// vk::raii::Device device;
	YAML::Node vk_config;
}; // namespace vis::vk

} // namespace vis::vulkan