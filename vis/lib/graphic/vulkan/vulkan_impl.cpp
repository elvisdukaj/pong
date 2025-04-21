module;

#include <yaml-cpp/yaml.h>

module vis.graphic.vulkan;

import std;
import vkh;
import vis.math;
import vis.window;

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
		std::swap(lhs.window, rhs.window);
		std::swap(lhs.context, rhs.context);
		std::swap(lhs.vk_instance, rhs.vk_instance);
		std::swap(lhs.surface, rhs.surface);
		std::swap(lhs.physical_devices, rhs.physical_devices);
		std::swap(lhs.selected_physical_device, rhs.selected_physical_device);
		std::swap(lhs.device, rhs.device);
		std::swap(lhs.command_pool, rhs.command_pool);
		std::swap(lhs.buffer_textures, rhs.buffer_textures);
		std::swap(lhs.render_pass, rhs.render_pass);
		std::swap(lhs.vk_config, rhs.vk_config);
	}

	std::string show_info() const {
		std::stringstream ss;
		ss << vk_config;
		return ss.str();
	}

	void set_clear_color(vec4 color) {
		clear_color = color;
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
		surface = vkh::SurfaceBuilder{vk_instance, window}.build();
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

		// clang-format off
		command_pool = vkh::CommandPoolBuilder{device}
				.with_queue_family_index(0)
				.create();
		// clang-format on

		std::shared_ptr<vkh::Texture> front_buffer;
		std::shared_ptr<vkh::Texture> back_buffer;
		std::shared_ptr<vkh::Texture> stencil_buffer;
		std::shared_ptr<vkh::Texture> depth_buffer;

		buffer_textures.emplace_back(std::move(front_buffer));
		buffer_textures.emplace_back(std::move(back_buffer));
		buffer_textures.emplace_back(std::move(stencil_buffer));
		buffer_textures.emplace_back(std::move(depth_buffer));

		// clang-format off
		// render_pass = vkh::RenderPassBuilder{device}
				// .add_attachments(buffer_textures)
				// .build();
		// clang-format on
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
	std::vector<std::shared_ptr<vkh::Texture>> buffer_textures;
	vkh::RenderPass render_pass{nullptr};
	YAML::Node vk_config;

	vis::vec4 clear_color{0.0f, 0.0f, 0.0f, 1.0f};
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

void Renderer::set_clear_color(vec4 color) {
	impl->set_clear_color(color);
}
void Renderer::clear() {}
void Renderer::set_viewport([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int width,
														[[maybe_unused]] int height) {}

} // namespace vis::vulkan