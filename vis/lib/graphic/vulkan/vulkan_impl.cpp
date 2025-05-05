module;

module vis.graphic.vulkan;

import std;
import vis.graphic.vulkan.vkh;
import vis.math;
import vis.window;

namespace helper {
constexpr vkh::InstanceCreateFlags get_required_instance_flags() noexcept {
  vkh::InstanceCreateFlags flags;

#if defined(__APPLE__)
  // flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
  flags |= vkh::InstanceCreateFlagBits::EnumeratePortabilityKHR;
#endif
  return flags;
}

std::vector<const char*> get_required_extensions() noexcept {
  std::vector<const char*> required_extensions{
      vkh::KHRGetPhysicalDeviceProperties2ExtensionName,
      vkh::KHRGetSurfaceCapabilities2ExtensionName,
  };

#if defined(__APPLE__)
  required_extensions.push_back(vkh::EXTMetalSurfaceExtensionName);
  required_extensions.push_back(vkh::KHRPortabilityEnumerationExtensionName);
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

} // namespace helper

namespace vis::vulkan {

#if 0
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
		auto feature10 = vkh::PhysicalDeviceFeatures2{};
		feature10.features.tessellationShader = vkh::True;

		auto feature12 = vkh::PhysicalDeviceVulkan12Features{};
		feature12.descriptorIndexing = vkh::True;
		feature12.bufferDeviceAddress = vkh::True;
		feature12.drawIndirectCount = vkh::True;
		feature12.samplerMirrorClampToEdge = vkh::True;
		feature12.storageBuffer8BitAccess = vkh::True;
		feature12.uniformBufferStandardLayout = vkh::True;
		feature12.shaderSubgroupExtendedTypes = vkh::True;
		feature12.shaderOutputLayer = vkh::True;
		feature12.vulkanMemoryModelAvailabilityVisibilityChains = vkh::True;
		feature12.shaderOutputViewportIndex = vkh::True;
		feature12.subgroupBroadcastDynamicId = vkh::True;
		feature12.separateDepthStencilLayouts = vkh::True;
		feature12.runtimeDescriptorArray = vkh::True;
		feature12.drawIndirectCount = vkh::True;
		feature12.drawIndirectCount = vkh::True;

		auto feature13 = vkh::PhysicalDeviceVulkan13Features{};
		feature13.synchronization2 = vkh::True;
		feature13.dynamicRendering = vkh::True;
		feature13.computeFullSubgroups = vkh::True;
		feature13.textureCompressionASTC_HDR = vkh::True;
		feature13.shaderZeroInitializeWorkgroupMemory = vkh::True;
		feature13.shaderIntegerDotProduct = vkh::True;
		feature13.maintenance4 = vkh::True;
		feature13.dynamicRendering = vkh::True;

		// clang-format off
		device = selected_physical_device
			.with_feature_10(feature10)
			.with_feature_12(feature12)
		  .with_feature_13(feature13)
			.create_device(vk_instance);

		command_pool = vkh::CommandPoolBuilder{device}
				.with_queue_family_index(0)
				.create();
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

	struct FrameData {
		vkh::Semaphore swapchain_sem, render_sem;
		vkh::Fence render_fence;
	};

	FrameData frame_data[2];

	vis::vec4 clear_color{0.0f, 0.0f, 0.0f, 1.0f};
	int width = 0;
	int height = 0;
};
#else
class Renderer::Impl {
public:
  Impl([[maybe_unused]] Window* window) : window{window} {
    create_instance();
    create_surface();
  }

  std::string show_info() const noexcept {
    return {};
  }

  void set_viewport([[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int width,
                    [[maybe_unused]] int height) noexcept {}

  void set_clear_color([[maybe_unused]] vec4 color) noexcept {}

private:
  void create_instance() {
    auto required_flags = helper::get_required_instance_flags();
    auto required_extensions = helper::get_required_extensions();
    auto required_layers = helper::get_required_layers();
    auto required_windows_extensions = window->get_required_renderer_extension();

    vk_instance = vkh::InstanceBuilder{vk_context}
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

    // vk_config["context"] = context.serialize();
    // for (const auto& required_extension : required_extensions) {
    // vk_config["context"]["required extensions"].push_back(required_extension);
    // }
    // for (const auto& required_extension : required_windows_extensions) {
    // vk_config["context"]["required windows extensions"].push_back(required_extension);
    // }
  }

  void create_surface() {
    surface = vkh::SurfaceBuilder{&vk_instance, window}.build();
  }

private:
  Window* window = nullptr;
  vkh::Context vk_context;
  vkh::Instance vk_instance{nullptr};
  vkh::Surface surface{nullptr};
};
#endif

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