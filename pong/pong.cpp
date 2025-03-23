module;

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>

export module game:app;

import :events;
import :components;
import :constants;
import :scene;
import :pong_scene;

import std;
import vis;

export namespace Game {
using namespace vis::literals::chrono_literals;

class App {
public:
	static App* create() {
		auto window = vis::Window::create("Pong", SCREEN_WIDTH, SCREEN_HEIGHT, screen_flags);
		if (not window)
			return nullptr;
		auto renderer = vis::opengl::OpenGLRenderer::create(window.value().get());
		if (not renderer)
			return nullptr;

		return new App{std::move(window.value()), std::move(renderer.value())};
	}

	[[nodiscard]] SDL_AppResult processEvent(const SDL_Event* event) noexcept {
		return pong_scene.processEvent(event);
	}

	[[nodiscard]] SDL_AppResult update() noexcept {
		return pong_scene.update();
	}

private:
	enum class IsPlayer : bool { yes = true, no = false };

	explicit App(vis::Window::Pointer window, vis::opengl::OpenGLRenderer::Pointer renderer)
			: window{std::move(window)}, renderer(std::move(renderer)), pong_scene{*this->renderer.get()} {}

private:
	vis::Window::Pointer window;
	vis::opengl::OpenGLRenderer::Pointer renderer;

	static constexpr SDL_WindowFlags screen_flags = SDL_WINDOW_OPENGL;

	PongScene pong_scene;
};

} // namespace Game
