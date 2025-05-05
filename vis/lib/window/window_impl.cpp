module;

#include <volk.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

module vis.window;

import std;

namespace vis {

Window::Window(std::string_view title, int width, int height, WindowsFlags flags) {
  using underlying = std::underlying_type<WindowsFlags>::type;
  volkInitialize();
  window = SDL_CreateWindow(title.data(), width, height, static_cast<underlying>(flags));
  if (not window)
    throw std::runtime_error(std::format("Unable to create window: {}", SDL_GetError()));
}

void swap(Window& lhs, Window& rhs) {
  ::std::swap(lhs.window, rhs.window);
}

Window::Window(Window&& other) : window{nullptr} {
  swap(*this, other);
}

Window& Window::operator=(Window&& other) {
  swap(*this, other);
  return *this;
}

Window::~Window() {
  if (window) {
    SDL_DestroyWindow(window);
    window = nullptr;
  }
}

VkSurfaceKHR Window::create_renderer_surface(VkInstance instance, const VkAllocationCallbacks* allocator) const {
  VkSurfaceKHR surface;
  if (not SDL_Vulkan_CreateSurface(window, instance, allocator, &surface)) {
    throw std::runtime_error("Unable to create the Vulkan Surface");
  }

  return surface;
}

std::vector<const char*> Window::get_required_renderer_extension() {
  Uint32 count_instance_extensions;
  const char* const* instance_extensions = SDL_Vulkan_GetInstanceExtensions(&count_instance_extensions);

  std::vector<const char*> required_windows_extensions;
  for (auto i = 0u; i != count_instance_extensions; ++i)
    required_windows_extensions.emplace_back(instance_extensions[i]);

  return required_windows_extensions;
}

} // namespace vis
