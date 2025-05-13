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

#if not defined(__linux__)
  required_layers.push_back("VK_LAYER_LUNARG_api_dump");
#endif

#endif

  return required_layers;
}

constexpr std::vector<const char*> get_physical_device_extensions() noexcept {
  std::vector<const char*> required_extensions = {vkh::KHRSwapchainExtensionName};

#if defined(__APPLE__)
  required_extensions.push_back(vkh::KHRPortabilitySubsetExtensionName);
#endif
  return required_extensions;
}

} // namespace helper

namespace vis::vulkan {

class Renderer::Impl {
public:
  Impl([[maybe_unused]] Window* window) : window{window} {
    init_instance();
    init_surface();
    auto physical_device_selector = vkh::PhysicalDeviceSelector{vk_instance, &surface};
    enumerate_physical_devices(physical_device_selector);
    init_device(physical_device_selector);
    init_swapchain();
    init_command_pool();
    init_semaphores();
    record_command_buffer();
  }

  std::string show_info() const noexcept {
    return {};
  }

  void set_viewport([[maybe_unused]] int x, [[maybe_unused]] int y, int view_width, int view_height) noexcept {
    width = view_width;
    height = view_height;
    init_swapchain();
  }

  void set_clear_color([[maybe_unused]] vec4 color) noexcept {
    clear_color = color;
  }

  void clear() const noexcept {
    const auto& swapchain_images = swapchain.get_images();
    for (auto i = 0uz; i < swapchain_images.size(); ++i) {
      const auto& image = swapchain_images[i];
      auto command_buffer = command_buffers[i];
      command_buffer.clear_color(clear_color, image, vkh::ImageLayout::transfer_dst_optimal, {});
    }
  }

  void draw() const noexcept {
    // clang-format off
    auto acquire_info = vkh::AcquireNextImageInfoKHRBuilder{swapchain}
                            .with_semaphore(image_availables_sem)
                            .build();
    // clang-format on

    auto swap_chain_image_index = swapchain.acquire_image(acquire_info);
    [[maybe_unused]] const auto& swapchain_images = swapchain.get_images();
    [[maybe_unused]] static const std::array<vkh::PipelineStageFlags, 1> wait_dst_flags = {
        vkh::PipelineStageFlagBits::transfer_bit};

    [[maybe_unused]] auto submits_info = std::vector<vkh::SubmitInfo>{
        vkh::SubmitInfoBuilder{}
            .add_wait_semaphore(image_availables_sem)
            .add_pipeline_flags(vkh::PipelineStageFlagBits::transfer_bit)
            .add_command_buffer(command_buffers[*swap_chain_image_index])
            .add_signal_semaphore(rendering_finished_sem)
            .build(),
    };

    // graphic_queue.submit(submits_info);

    // auto presents_info = vkh::PresentInfoBuilder{}
    //                          .add_wait_semaphore(rendering_finished_sem)
    //                          .add_image_index(*swap_chain_image_index)
    //                          .add_swapchain(swapchain)
    //                          .build();
    // present_queue.present(presents_info);
  }

private:
  void init_instance() noexcept {
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
                      // .with_maximum_required_instance_version(0, 1, 2, 0)
                      .build();
  }

  void init_surface() noexcept {
    surface = vkh::SurfaceBuilder{&vk_instance, window}.build();
  }

  void enumerate_physical_devices(vkh::PhysicalDeviceSelector& physical_device_selector) noexcept {
    auto required_gpu_extensions = helper::get_physical_device_extensions();

    // clang-format off
    physical_devices = physical_device_selector
                           .add_required_extensions(required_gpu_extensions)
                           .allow_gpu_type(vkh::PhysicalDeviceType::DiscreteGpu)
                           .allow_gpu_type(vkh::PhysicalDeviceType::IntegratedGpu)
                           .enumerate_all();
    // clang-format on
  }

  void init_device(vkh::PhysicalDeviceSelector& physical_device_selector) {
    selected_physical_device_it = physical_device_selector.select(begin(physical_devices), end(physical_devices));
    if (selected_physical_device_it == end(physical_devices))
      throw std::runtime_error{"No suitable physical device found"};

    present_queue_family_index = *selected_physical_device_it->get_first_graphic_and_present_queue_family_index();

    std::println("selected device: {}", selected_physical_device_it->device_name());

    // clang-format off
    auto present_queue_info_builder = vkh::DeviceQueueCreateInfoBuilder{}
          .with_family_index(present_queue_family_index);

    device = physical_device_selector
		      .with_queue(present_queue_info_builder.build())
		      .create_device(*selected_physical_device_it);
    // clang-format on

    graphic_queue = device.get_queue(present_queue_family_index);
    present_queue = device.get_queue(present_queue_family_index);

    auto surface_caps = selected_physical_device_it->get_surface_capabilities();
    width = static_cast<int>(surface_caps.surfaceCapabilities.currentExtent.width);
    height = static_cast<int>(surface_caps.surfaceCapabilities.currentExtent.height);
    swapchain_image_count =
        3; // std::min(static_cast<std::size_t>(surface_caps.surfaceCapabilities.maxImageCount), 3uz);
  }

  void init_swapchain() {
    // clang-format off
    swapchain = vkh::SwapchainBuilder{*selected_physical_device_it, device, surface}
      .with_extent(width, height)
      .with_required_format(vkh::Format::B8G8R8A8Srgb)
      .with_present_mode(vkh::PresentMode::fifo)
      .with_image_count(swapchain_image_count)
      .add_usage(vkh::ImageUsageFlagBits::color_attachment_bit)
      .add_usage(vkh::ImageUsageFlagBits::transfer_dst_bit)
      .with_old_swapchain(swapchain)
      .build();
    // clang-format on
  }

  void init_command_pool() {
    // clang-format off
    command_pool = vkh::CommandPoolBuilder{device}
      .with_queue_family_index(present_queue_family_index)
      .with_flags(vkh::CommandPoolCreateFlagBits::reset_command_buffer)
      .build();

    const auto& swapchain_images = swapchain.get_images();

    command_buffers = vkh::CommandBuffersBuilder{device, command_pool}
        .with_buffer_count(swapchain_images.size())
        .build();
    // clang-format on
  }

  void init_semaphores() {
    image_availables_sem = vkh::SemaphoreBuilder{device}.build();
    rendering_finished_sem = vkh::SemaphoreBuilder{device}.build();
  }

  void record_command_buffer() const {
    const auto& swapchain_images = swapchain.get_images();

    static const std::vector<vkh::ImageSubresourceRange> subresource_ranges = {
        vkh::ImageSubresourceRangeBuilder{}.with_aspect_mask(vkh::ImageAspectFlagBits::color_bit).build(),
    };

    for (auto i = 0uz; i < swapchain_images.size(); ++i) {
      const auto& image = swapchain_images[i];
      auto command_buffer = command_buffers[i];

      std::vector<vkh::ImageMemoryBarrier> barrier_from_present_to_clear = {
          vkh::ImageMemoryBarrierBuilder{}
              .with_src_access_mask(vkh::AccessFlagBits::memory_read_bit)
              .with_dst_access_mask(vkh::AccessFlagBits::transfer_write_bit)
              .with_old_layout(vkh::ImageLayout::undefined)
              .with_new_layout(vkh::ImageLayout::transfer_dst_optimal)
              .with_src_queue_family_index(present_queue_family_index)
              .with_dst_queue_family_index(present_queue_family_index)
              .with_image(image)
              .with_subresource_range(subresource_ranges.front())
              .build(),
      };

      std::vector<vkh::ImageMemoryBarrier> barrier_from_clear_to_present = {
          vkh::ImageMemoryBarrierBuilder{}
              .with_src_access_mask(vkh::AccessFlagBits::transfer_write_bit)
              .with_dst_access_mask(vkh::AccessFlagBits::memory_read_bit)
              .with_old_layout(vkh::ImageLayout::transfer_dst_optimal)
              .with_new_layout(vkh::ImageLayout::present_src_khr)
              .with_src_queue_family_index(present_queue_family_index)
              .with_dst_queue_family_index(present_queue_family_index)
              .with_image(image)
              .with_subresource_range(subresource_ranges.front())
              .build(),
      };

      static const auto begin_record_info = vkh::CommandBufferBeginInfoBuilder{}
                                                .with_flags(vkh::CommandBufferUsageFlagBits::simultaneous_use_bit)
                                                .build();

      command_buffer.start_recording(begin_record_info);

      command_buffer.pipeline_barrier(vkh::PipelineStageFlagBits::transfer_bit,
                                      vkh::PipelineStageFlagBits::transfer_bit, barrier_from_present_to_clear);

      command_buffer.clear_color(clear_color, image, vkh::ImageLayout::transfer_dst_optimal, subresource_ranges);

      command_buffer.pipeline_barrier(vkh::PipelineStageFlagBits::transfer_bit,
                                      vkh::PipelineStageFlagBits::bottomo_of_pipe_bit, barrier_from_clear_to_present);

      command_buffer.end_recording();
    }
  }

private:
  Window* window = nullptr;
  vkh::Context vk_context;
  vkh::Instance vk_instance{nullptr};
  vkh::Surface surface{nullptr};
  std::vector<vkh::PhysicalDevice> physical_devices;
  std::vector<vkh::PhysicalDevice>::iterator selected_physical_device_it;
  vkh::Device device{nullptr};
  vkh::Queue graphic_queue{};
  vkh::Queue present_queue{};
  vkh::Swapchain swapchain{nullptr};
  vkh::CommandPool command_pool{nullptr};
  vkh::CommandBuffers command_buffers{};
  vkh::Semaphore image_availables_sem{nullptr};
  vkh::Semaphore rendering_finished_sem{nullptr};

  vis::vec4 clear_color{0.0f, 0.0f, 0.0f, 1.0f};
  int width = 800;
  int height = 600;
  std::size_t present_queue_family_index = 0;
  std::size_t swapchain_image_count = 0;
};

Renderer::Renderer(Window* window) : impl{std::make_unique<Renderer::Impl>(window)} {}
Renderer::~Renderer() = default;

Renderer::Renderer(Renderer&&) = default;
Renderer& Renderer::operator=(Renderer&&) = default;

std::string Renderer::show_info() const noexcept {
  return impl->show_info();
}

void Renderer::render() noexcept {
  impl->draw();
  // SDL_Vulkan
  // SDL_GL_SwapWindow(*context.window);
}

void Renderer::set_clear_color(vec4 color) noexcept {
  impl->set_clear_color(color);
}

void Renderer::clear() noexcept {
  impl->clear();
}

void Renderer::set_viewport(int x, int y, int width, int height) noexcept {
  impl->set_viewport(x, y, width, height);
}

} // namespace vis::vulkan