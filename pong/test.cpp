module;

export module game:test_scene;

import :events;
import :components;
import :constants;
import :scene;

import std;
import vis;

// helper type for the visitor
template <class... Ts> struct overloads : Ts... {
  using Ts::operator()...;
};

export {
  namespace Game {
  using namespace vis::literals::chrono_literals;

  class TestScene : public Scene {
  public:
    explicit TestScene(vis::vulkan::Renderer& renderer) : renderer(renderer) {}

    [[nodiscard]] vis::app::AppResult process_event(const vis::win::Event& event) noexcept override {
      return std::visit(
          overloads{
              [&](const vis::win::QuitEvent&) { return vis::app::AppResult::success; },
              [&](const vis::win::KeyboardKeyDownEvent& event) {
                if (event.key == vis::win::VirtualKey::escape) {
                  return vis::app::AppResult::success;
                }
                return vis::app::AppResult::app_continue;
              },
              [&](const vis::win::KeyboardKeyUpEvent&) { return vis::app::AppResult::app_continue; },
              [&](const vis::win::WindowsResized& event) {
                screen_width = event.width;
                screen_height = event.height;
                renderer.set_viewport(0, 0, screen_width, screen_height);
                screen_proj = vis::orthogonal_matrix(screen_width, screen_height, world_width, world_height);
                return vis::app::AppResult::app_continue;
              },
              [&]([[maybe_unused]] const auto& all_other_events) { return vis::app::AppResult::app_continue; },
          },
          event);
    }

    [[nodiscard]] vis::app::AppResult update() noexcept override {
      renderer.render();
      return vis::app::AppResult::app_continue;
    }

  private:
    void initialize_video() {
      std::println("{}", renderer.show_info());
      renderer.set_clear_color(colors::orange);
      renderer.set_viewport(0, 0, screen_width, screen_height);
      screen_proj = vis::orthogonal_matrix(screen_width, screen_height, world_width, world_height);
    }

  private:
    vis::vulkan::Renderer& renderer;

    int screen_width = SCREEN_WIDTH;
    int screen_height = SCREEN_HEIGHT;

    vis::ScreenProjection screen_proj;
  };

  } // namespace Game
}
