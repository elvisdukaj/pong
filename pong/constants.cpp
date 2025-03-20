module;

export module game:constants;

import vis;
import std;

export namespace Game {

constexpr vis::vec2 up{0.0f, 1.0f};
constexpr vis::vec2 down{0.0f, -1.0f};
constexpr auto origin = vis::vec2{0.0f, 0.0f};

constexpr int SCREEN_HEIGHT = 600;
constexpr int SCREEN_WIDTH = 800;

constexpr std::ratio<4, 3> ASPECT_RATIO;

constexpr auto pad_thickness = 0.6f;
constexpr auto pad_length = 3.0f;
constexpr auto ball_radius = 0.4f;

constexpr auto half_wall_thickness = 0.3f;
constexpr auto initial_player_speed = 20.0f;
constexpr auto initial_ai_speed = 20.0f;
constexpr auto ball_acceleration_magnitude = 2.0f;

constexpr auto world_width = 20.0f;
constexpr auto world_height = 20.0f * static_cast<float>(ASPECT_RATIO.num) / static_cast<float>(ASPECT_RATIO.den);

constexpr auto friction = 0.01f;

namespace colors {
constexpr auto black = vis::vec4{0.0f, 0.0f, 0.0f, 1.0f};
constexpr auto red = vis::vec4{1.0f, 0.0f, 0.0f, 1.0f};
constexpr auto white = vis::vec4{1.0f, 1.0f, 1.0f, 1.0f};
} // namespace colors

} // namespace Game
