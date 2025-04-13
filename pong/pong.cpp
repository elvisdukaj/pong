module;

export module game:app;

import :events;
import :components;
import :constants;
import :scene;
import :pong_scene;
import :test_scene;

import std;
import vis;

export namespace Game {
using namespace vis::literals::chrono_literals;

class App {
public:
	static App* create() {
		auto window = vis::Window::create("Pong", SCREEN_WIDTH, SCREEN_HEIGHT, screen_flags);
		if (not window) {
			std::println("Unable to create the window! An error occured: {}", window.error());
			return nullptr;
		}

		// static auto renderer = vis::gl::OpenGLRenderer::create(&(window.value()));
		// if (not renderer) {
		// 	std::println("Unable to create the window! An error occured: {}", window.error());
		// 	return nullptr;
		// }

		auto vk_renderer = vis::vulkan::Renderer::create(&(window.value()));
		if (not vk_renderer) {
			std::println("Unable to create the Vulkan Renderer! An error occured: {}", vk_renderer.error());
			return nullptr;
		}

		std::println("{}", vk_renderer->show_info());

		static auto app = new App{std::move(window.value()), std::move(vk_renderer.value())};
		return app;
	}

	[[nodiscard]] vis::app::AppResult process_event(const vis::win::Event& event) noexcept {
		return test_scene.process_event(event);
	}

	[[nodiscard]] vis::app::AppResult update() noexcept {
		return test_scene.update();
	}

private:
	explicit App(vis::Window&& window, vis::vulkan::Renderer&& renderer)
			: window{std::move(window)}, renderer(std::move(renderer)), test_scene{this->renderer} {}

private:
	vis::Window window;
	// vis::opengl::OpenGLRenderer* renderer;
	vis::vulkan::Renderer renderer;
	static constexpr vis::WindowsFlags screen_flags = vis::WindowsFlags::vulkan;

	TestScene test_scene;
};

} // namespace Game
