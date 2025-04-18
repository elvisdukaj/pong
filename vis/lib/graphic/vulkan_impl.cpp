module;

#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#include <SDL3/SDL.h>

#include <yaml-cpp/yaml.h>

#include <cassert>

module vis.graphic.vulkan;

import std;

import vkh;
import vis.math;
import vis.window;

namespace views = std::ranges::views;

namespace vis::vulkan {

class Renderer::Impl {
public:
	Impl(Window* window) : window{window} {
		create_instance();
		create_surface();

		vkh::PhysicalDeviceSelector device_selector{vk_instance, &surface};
		enumerate_gpus(device_selector);
		select_gpu(device_selector);
		create_device_and_command_pool();
	}

	friend void swap(Renderer::Impl& lhs, Renderer::Impl& rhs) {
		// std::swap(lhs.required_extensions, rhs.required_extensions);
		// std::swap(lhs.required_layers, rhs.required_layers);
		std::swap(lhs.window, rhs.window);
		std::swap(lhs.context, rhs.context);
		std::swap(lhs.vk_instance, rhs.vk_instance);
		std::swap(lhs.surface, rhs.surface);
		std::swap(lhs.physical_devices, rhs.physical_devices);
		std::swap(lhs.selected_physical_device, rhs.selected_physical_device);
		std::swap(lhs.device, rhs.device);
		std::swap(lhs.command_pool, rhs.command_pool);
		// std::swap(lhs.transfer_queue_index, rhs.transfer_queue_index);
		// std::swap(lhs.gpu_score, rhs.gpu_score);
		// std::swap(lhs.gpu_queue_index, rhs.gpu_queue_index);
		// std::swap(lhs.device, rhs.device);
		std::swap(lhs.vk_config, rhs.vk_config);
	}

	std::string show_info() const {
		std::stringstream ss;
		ss << vk_config;
		return ss.str();
	}

private:
	void create_instance() {
		auto required_flags = vkh::get_required_instance_flags();
		auto required_extensions = vkh::get_required_extensions();
		auto required_layers = vkh::get_required_layers();
		auto required_windows_extensions = window->get_required_renderer_extension();

		vk_instance = vkh::InstanceBuilder{context}
											.with_app_name("Pong")
											.with_app_version(0, 1, 1)
											.with_engine_name("vis")
											.with_engine_version(0, 1, 1)
											.with_app_flags(required_flags)
											.add_required_layers(required_layers)
											.add_required_extensions(required_extensions)
											.add_required_extensions(required_windows_extensions)
											.build();

		vk_config["instance"] = context.serialize();
		for (const auto& required_extension : required_extensions) {
			vk_config["instance"]["required extensions"].push_back(required_extension);
		}
		for (const auto& required_extension : required_windows_extensions) {
			vk_config["instance"]["required windows extensions"].push_back(required_extension);
		}
	}

	void create_surface() {
		const auto& cpp_instance = static_cast<vk::Instance>(vk_instance);
		auto c_instance = static_cast<VkInstance>(cpp_instance);

		VkSurfaceKHR vk_surface = window->create_renderer_surface(c_instance, nullptr);
		surface = vkh::Surface{vk_instance, vk_surface, nullptr};
	}

	void enumerate_gpus(vkh::PhysicalDeviceSelector& device_selector) {
		auto nodes = vk_config["physical devices"];
		for (auto&& physical_device : device_selector.enumerate_all()) {
			nodes.push_back(physical_device.dump());
		}
	}

	void select_gpu(vkh::PhysicalDeviceSelector& device_selector) {
		device_selector.with_surface(&surface);

		auto required_gpu_extensions = vkh::get_physical_device_extensions();
		vk_config["physical device"]["required extensions"] = required_gpu_extensions;

		// clang-format off
		auto expected_device = device_selector
				.add_required_extensions(required_gpu_extensions)
				.with_surface(&surface)
				.allow_discrete_device()
				.allow_integrate_device()
				.set_require_preset()
				.set_require_compute()
				.set_require_transfer()
				.select();
		// clang-format on

		if (not expected_device) {
			throw std::runtime_error{expected_device.error()};
		}

		selected_physical_device = *expected_device;
		vk_config["selected physical device"] = selected_physical_device.name();
	}

	void create_device_and_command_pool() {
		device = selected_physical_device.create_device();
		command_pool = vkh::CommandPoolBuilder{device}.with_queue_family_index(0).create();
	}

private:
	Window* window = nullptr;
	vkh::Context context;
	vkh::Instance vk_instance{nullptr};
	vkh::Surface surface{nullptr};
	std::vector<vkh::PhysicalDevice> physical_devices;
	vkh::PhysicalDevice selected_physical_device;
	vkh::Device device{nullptr};
	vkh::CommandPool command_pool{nullptr};
	YAML::Node vk_config;
};

std::expected<Renderer, std::string> Renderer::create(Window* window) {
	try {
		return Renderer{window};
	} catch (const std::exception& exc) {
		return std::unexpected{exc.what()};
	} catch (...) {
		return std::unexpected{"Unable to create a Vulkan instance"};
	}
}

Renderer::Renderer(Window* window) : impl{std::make_unique<Renderer::Impl>(window)} {}
Renderer::~Renderer() = default;

Renderer::Renderer(Renderer&&) = default;
Renderer& Renderer::operator=(Renderer&&) = default;

std::string Renderer::show_info() const {
	return impl->show_info();
}

void Renderer::render() const {
	// SDL_Vulka
	// SDL_GL_SwapWindow(*context.window);
}

void Renderer::set_clear_color([[maybe_unused]] const vec4& color) {}
void Renderer::clear() {}
void Renderer::set_viewport([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int width,
														[[maybe_unused]] int height) {}

} // namespace vis::vulkan