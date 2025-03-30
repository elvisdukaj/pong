module;

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
		static auto window = vis::Window::create("Pong", SCREEN_WIDTH, SCREEN_HEIGHT, screen_flags);
		if (not window)
			return nullptr;

		static auto renderer = vis::gl::OpenGLRenderer::create(&(window.value()));
		if (not renderer)
			return nullptr;

		static auto vk_renderer = vis::vk::Renderer::create(&(window.value()));
		if (not vk_renderer)
			return nullptr;

		std::println( "{}", vk_renderer->show_info());

		static auto app = new App{/*&(window.value()),*/ &(renderer.value())};
		return app;
	}

	[[nodiscard]] vis::app::AppResult process_event(const vis::win::Event& event) noexcept {
		return pong_scene.process_event(event);
	}

	[[nodiscard]] vis::app::AppResult update() noexcept {
		return pong_scene.update();
	}

private:
	explicit App(/*vis::Window* window,*/ vis::gl::OpenGLRenderer* renderer)
			: /*window{window}, renderer(renderer),*/ pong_scene{*renderer} {}

private:
	// vis::Window* window;
	// vis::opengl::OpenGLRenderer* renderer;

	static constexpr vis::WindowsFlags screen_flags = vis::WindowsFlags::opengl;

	PongScene pong_scene;
};

} // namespace Game
