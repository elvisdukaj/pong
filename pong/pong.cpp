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
    try {
      static auto app = new App{TITLE, SCREEN_WIDTH, SCREEN_HEIGHT};
      return app;
    } catch (const std::exception& exc) {
      std::println("Unable to create the Vulkan Renderer! An error occured: {}", exc.what());
    } catch (...) {
      std::println("Unable to create the Vulkan Renderer! An error occured");
    }
    return nullptr;
  }

  [[nodiscard]] vis::app::AppResult process_event(const vis::win::Event& event) noexcept {
    return test_scene.process_event(event);
  }

  [[nodiscard]] vis::app::AppResult update() noexcept {
    return test_scene.update();
  }

private:
  App(std::string_view title, int width, int height)
      : window{title, width, height, screen_flags}, renderer{&window}, test_scene{renderer} {
    renderer.set_viewport(0, 0, width, height);
    std::println("{}", renderer.show_info());
  }

private:
  vis::Window window;
  vis::vulkan::Renderer renderer;
  static constexpr vis::WindowsFlags screen_flags = vis::WindowsFlags::vulkan;

  TestScene test_scene;
};

} // namespace Game
