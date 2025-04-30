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
		select_gpu(device_selector);
		create_device_and_command_pool();
	}

	void swap(Renderer::Impl& other) noexcept {
		std::swap(window, other.window);
		std::swap(context, other.context);
		std::swap(vk_instance, other.vk_instance);
		std::swap(surface, other.surface);
		std::swap(physical_devices, other.physical_devices);
		std::swap(selected_physical_device, other.selected_physical_device);
		std::swap(device, other.device);
		std::swap(command_pool, other.command_pool);
		std::swap(buffer_textures, other.buffer_textures);
		std::swap(render_pass, other.render_pass);
		std::swap(vk_config, other.vk_config);
	}

	std::string show_info() const {
		std::stringstream ss;
		ss << vk_config;
		return ss.str();
	}

	void set_clear_color(vec4 color) noexcept {
		clear_color = color;
	}

	void set_viewport([[maybe_unused]] int x, [[maybe_unused]] int y, int view_width, int view_height) noexcept {
		width = view_width;
		height = view_height;
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
											.with_minimum_required_instance_version(0, 1, 2, 0)
											.with_maximum_required_instance_version(0, 1, 2, 0)
											.build();

		vk_config["context"] = context.serialize();
		for (const auto& required_extension : required_extensions) {
			vk_config["context"]["required extensions"].push_back(required_extension);
		}
		for (const auto& required_extension : required_windows_extensions) {
			vk_config["context"]["required windows extensions"].push_back(required_extension);
		}
	}

	void create_surface() {
		surface = vkh::SurfaceBuilder{vk_instance, window}.build();
	}

	void select_gpu(vkh::PhysicalDeviceSelector& device_selector) {
		device_selector.with_surface(&surface);

		auto required_gpu_extensions = vkh::get_physical_device_extensions();
		// vk_config["physical device"]["required extensions"] = required_gpu_extensions;

		// clang-format off
		auto expected_device = device_selector
				.add_required_extensions(required_gpu_extensions)
#if not defined(NDEBUG)
				.add_required_layer("VK_LAYER_KHRONOS_validation")
#endif
				.with_surface(&surface)
				.allow_discrete_device()
				.allow_integrate_device()
				.set_require_preset()
				.set_require_compute()
				.set_require_transfer()
				.select();
		// clang-format on

		selected_physical_device = std::move(expected_device);
		vk_config["selected physical device"] = selected_physical_device.dump();
	}

	void create_swapchain() {
		auto builder = vkh::SwapChainBuilder{surface, device};
		// clang-format off
		swapchain =
			builder.set_extent(width, height)
			.set_image_usage(vkh::ImageUsageFlagBits::eTransferDst)
			.set_present_mode(vkh::PresentModeKHR::eFifo)
			.set_required_format(vkh::Format::eB8G8R8A8Unorm)
			.set_required_color_space(vkh::ColorSpaceKHR::eSrgbNonlinear)
			.build();
		// clang-format on
	}

	void create_device_and_command_pool() {
		device = selected_physical_device.create_device(vk_instance);

		// clang-format off
		// command_pool = vkh::CommandPoolBuilder{device}
				// .with_queue_family_index(0)
				// .create();
		// clang-format on

#if HAS_RENDER_PASS
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
#endif
	}

private:
	Window* window = nullptr;
	vkh::Context context;
	vkh::Instance vk_instance{nullptr};
	vkh::Surface surface{nullptr};
	std::vector<vkh::PhysicalDevice> physical_devices;
	vkh::PhysicalDevice selected_physical_device;
	vkh::Device device{nullptr};
	vkh::SwapChain swapchain{nullptr};
	vkh::CommandPool command_pool{nullptr};
	// vkh::CommandPool graphic_command_pool;
	// vkh::CommandPool preset_command_pool;
	// vkh::CommandPool compute_command_pool;
	// vkh::CommandPool transfer_command_pool;
	std::vector<std::shared_ptr<vkh::Texture>> buffer_textures;
	vkh::RenderPass render_pass{nullptr};
	YAML::Node vk_config;

	vis::vec4 clear_color{0.0f, 0.0f, 0.0f, 1.0f};
	int width = 0;
	int height = 0;
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

std::string Renderer::show_info() const noexcept {
	return impl->show_info();
}

void Renderer::render() noexcept {
	// SDL_Vulkan
	// SDL_GL_SwapWindow(*context.window);
}

void Renderer::set_clear_color(vec4 color) noexcept {
	impl->set_clear_color(color);
}

void Renderer::clear() noexcept {}

void Renderer::set_viewport(int x, int y, int width, int height) noexcept {
	impl->set_viewport(x, y, width, height);
}

} // namespace vis::vulkan