module;

#include <SDL3/SDL.h>
#include <memory>
#include <optional>

export module vis:window;

import std;

export namespace vis {

class Window {
public:
	using Pointer = std::unique_ptr<Window>;
	static Pointer create(std::string_view title, int width, int height, SDL_WindowFlags flags) {
		auto window = SDL_CreateWindow(title.data(), width, height, flags);
		if (not window)
			return nullptr;

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