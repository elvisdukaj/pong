module;

/* Avoid including vulkan.h, don't define VkInstance if it's already included */
#ifdef VULKAN_H_
#define NO_SDL_VULKAN_TYPEDEFS
#endif
#ifndef NO_SDL_VULKAN_TYPEDEFS
#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;

#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64) ||               \
    defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T* object;
#else
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#endif

VK_DEFINE_HANDLE(VkInstance)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)
struct VkAllocationCallbacks;
#endif
#undef VK_DEFINE_HANDLE
#undef VK_DEFINE_NON_DISPATCHABLE_HANDLE

extern "C" {
struct SDL_Window;
}

export module vis.window;

import std;

export namespace vis {

enum class WindowsFlags : std::uint64_t {
  fullscreen = 0x1,
  opengl = 0x02,
  occluded = 0x04,
  hidden = 0x08,
  borderless = 0x10,
  resizable = 0x20,
  minimized = 0x40,
  maximized = 0x80,
  grabbed = 0x100,
  input_focus = 0x200,
  mouse_focus = 0x400,
  external = 0x800,
  high_pixel_density = 0x1000,
  modal = 0x2000,
  mouse_capture = 0x4000,
  mouse_relative_mode = 0x8000,
  always_on_top = 0x10000,
  utility = 0x20000,
  tooltip = 0x40000,
  popup_menu = 0x80000,
  keyboard_grabbed = 0x100000,
  vulkan = 0x0000000010000000,
  metal = 0x400000,
  transparent = 0x0000000040000000,
  not_focusable = 0x0000000080000000,
};

class Window {
public:
  Window(std::string_view title, int width, int height, WindowsFlags flags);

  Window(Window&) = delete;
  Window& operator=(Window&) = delete;

  Window(Window&&);
  Window& operator=(Window&& rhs);

  ~Window();

  VkSurfaceKHR create_renderer_surface(VkInstance instance, const VkAllocationCallbacks* allocator) const;
  std::vector<const char*> get_required_renderer_extension();

  explicit operator SDL_Window*() const noexcept {
    return window;
  }

private:
  friend void swap(Window& lhs, Window& rhs);

private:
  SDL_Window* window = nullptr;
};

} // namespace vis

export namespace vis::win {

enum class Pressed : bool { no = false, yes = true };
enum class Repeated : bool { no = false, yes = true };
enum class VirtualKey : std::uint64_t {
  up = 0x40000052u,
  down = 0x40000051u,
  left = 0x40000050u,
  right = 0x4000004fu,
  escape = 0x0000001bu,
};
enum class KeyMode : std::uint32_t {};

struct KeyboardKeyDownEvent {
  VirtualKey key; /**< SDL virtual key code */
  // KeyMode mod;		/**< current key modifiers */

  Pressed pressed;
  Repeated repeated;
};

struct KeyboardKeyUpEvent {
  VirtualKey key; /**< SDL virtual key code */
  // KeyMode mod;		/**< current key modifiers */

  Pressed pressed;
  Repeated repeated;
};

struct QuitEvent {};

struct NullEvent {};

struct WindowsResized {
  int width;
  int height;
};

using Event = std::variant<KeyboardKeyDownEvent, KeyboardKeyUpEvent, QuitEvent, WindowsResized, NullEvent>;

} // namespace vis::win
