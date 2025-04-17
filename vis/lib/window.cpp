module;

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

export module vis.window;

import std;

export namespace vis {

enum class WindowsFlags : uint64_t {
	fullscreen = SDL_WINDOW_FULLSCREEN,
	opengl = SDL_WINDOW_OPENGL,
	occluded = SDL_WINDOW_OCCLUDED,
	hidden = SDL_WINDOW_HIDDEN,
	borderless = SDL_WINDOW_BORDERLESS,
	resizable = SDL_WINDOW_RESIZABLE,
	minimized = SDL_WINDOW_MINIMIZED,
	maximized = SDL_WINDOW_MAXIMIZED,
	grabbed = SDL_WINDOW_MOUSE_GRABBED,
	input_focus = SDL_WINDOW_INPUT_FOCUS,
	mouse_focus = SDL_WINDOW_MOUSE_FOCUS,
	external = SDL_WINDOW_EXTERNAL,
	high_pixel_density = SDL_WINDOW_HIGH_PIXEL_DENSITY,
	modal = SDL_WINDOW_MODAL,
	mouse_capture = SDL_WINDOW_MOUSE_CAPTURE,
	mouse_relative_mode = SDL_WINDOW_MOUSE_RELATIVE_MODE,
	always_on_top = SDL_WINDOW_ALWAYS_ON_TOP,
	utility = SDL_WINDOW_UTILITY,
	tooltip = SDL_WINDOW_TOOLTIP,
	popup_menu = SDL_WINDOW_POPUP_MENU,
	keyboard_grabbed = SDL_WINDOW_KEYBOARD_GRABBED,
	vulkan = SDL_WINDOW_VULKAN,
	metal = SDL_WINDOW_METAL,
	transparent = SDL_WINDOW_TRANSPARENT,
	not_focusable = SDL_WINDOW_NOT_FOCUSABLE,
	window_pos_undefined = SDL_WINDOWPOS_UNDEFINED,
	window_pos_centered = SDL_WINDOWPOS_CENTERED,
};

constexpr WindowsFlags operator|(WindowsFlags lhs, WindowsFlags rhs) {
	using underlying = std::underlying_type<WindowsFlags>::type;
	return static_cast<WindowsFlags>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
}

class Window {
public:
	static std::expected<Window, std::string> create(std::string_view title, int width, int height, WindowsFlags flags) {
		using underlying = std::underlying_type<WindowsFlags>::type;
		auto window = SDL_CreateWindow(title.data(), width, height, static_cast<underlying>(flags));
		if (not window)
			return std::unexpected(std::format("Unable to create window: {}", SDL_GetError()));

		return Window{window};
	}

	~Window() {
		if (window) {
			SDL_DestroyWindow(window);
			window = nullptr;
		}
	}

	friend void swap(Window& lhs, Window& rhs) {
		std::swap(lhs.window, rhs.window);
	}

	Window(Window&) = delete;
	Window& operator=(Window&) = delete;

	Window(Window&& rhs) noexcept : window{nullptr} {
		swap(*this, rhs);
	}

	Window& operator=(Window&& rhs) noexcept {
		swap(*this, rhs);
		return *this;
	};

	explicit operator SDL_Window*() const {
		return window;
	}

	VkSurfaceKHR create_renderer_surface(VkInstance instance, const VkAllocationCallbacks* allocator) const {
		VkSurfaceKHR surface;
		if (not SDL_Vulkan_CreateSurface(window, instance, allocator, &surface)) {
			throw std::runtime_error("Unable to create the Vulkan Surface");
		}

		return surface;
	}

	std::vector<const char*> get_required_renderer_extension() {
		Uint32 count_instance_extensions;
		const char* const* instance_extensions = SDL_Vulkan_GetInstanceExtensions(&count_instance_extensions);

		std::vector<const char*> required_windows_extensions;
		for (auto i = 0u; i != count_instance_extensions; ++i)
			required_windows_extensions.emplace_back(instance_extensions[i]);

		return required_windows_extensions;
	}

private:
	explicit Window(SDL_Window* window) : window(window) {
		SDL_ShowWindow(window);
	}

private:
	SDL_Window* window;
};

} // namespace vis

export namespace vis::win {

enum class Pressed : bool { no = false, yes = true };
enum class Repeated : bool { no = false, yes = true };
enum class VirtualKey { up = SDLK_UP, down = SDLK_DOWN, left = SDLK_LEFT, right = SDLK_RIGHT, escape = SDLK_ESCAPE };
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

using vis::operator|;