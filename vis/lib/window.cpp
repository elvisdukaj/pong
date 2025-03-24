module;

#include <SDL3/SDL.h>

export module vis:window;

import std;

export namespace vis {

class Window {
public:
	using Pointer = std::unique_ptr<Window>;
	static std::expected<Pointer, std::string> create(std::string_view title, int width, int height,
																										SDL_WindowFlags flags) {
		auto window = SDL_CreateWindow(title.data(), width, height, flags);
		if (not window)
			return std::unexpected(std::format("Unable to create window: {}", SDL_GetError()));

		return Pointer{new Window{window}};
	}

	~Window() {
		if (window) {
			SDL_DestroyWindow(window);
			window = nullptr;
		}
	}

	Window(Window&) = delete;
	Window& operator=(Window&) = delete;

	Window(Window&& rhs) : window{rhs.window} {
		rhs.window = nullptr;
	}

	Window& operator=(Window&& rhs) {
		window = rhs.window;
		rhs.window = nullptr;
		return *this;
	};

	operator SDL_Window*() {
		return window;
	}

private:
	Window(SDL_Window* window) : window(window) {
		SDL_ShowWindow(window);
	}

private:
	SDL_Window* window;
};

} // namespace vis

export namespace vis::win {

enum class Pressed : bool { no = false, yes = true };
enum class Repeated : bool { no = false, yes = true };
enum class VirtualKey { up = SDLK_UP, down = SDLK_DOWN, escape = SDLK_ESCAPE };
enum class KeyMode : std::uint32_t {};

struct KeyboardEvent {
	enum Type { key_down = SDL_EVENT_KEY_DOWN, key_up = SDL_EVENT_KEY_DOWN };

	Type type;

	VirtualKey key; /**< SDL virtual key code */
	// KeyMode mod;		/**< current key modifiers */

	Pressed pressed;
	Repeated repeated;
};
struct QuitEvent {};

using Event = std::variant<KeyboardEvent, QuitEvent>;

} // namespace vis::win