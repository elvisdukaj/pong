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

using ASPECT_RATIO = std::ratio<4, 3>;
// constexpr std::ratio<4, 3> ASPECT_RATIO;

constexpr auto pad_thickness = 0.1f;
constexpr auto pad_length = .4f;
constexpr auto half_pad_length = pad_length / 2.0f;
constexpr auto ball_radius = 0.06f;
constexpr auto wall_thickness = pad_thickness;
constexpr auto half_wall_thickness = wall_thickness / 2.0f;

constexpr auto initial_player_speed = 3.0f;
constexpr auto initial_ai_speed = initial_player_speed;
constexpr auto ball_vel_min_speed = 2.1f;
constexpr auto ball_vel_max_speed = 5.2f;
constexpr auto ball_angle_min = vis::radians(-30.0f);
constexpr auto ball_angle_max = vis::radians(+30.0f);

constexpr auto world_width = 2.0f;
constexpr auto world_height =
		world_width * static_cast<float>(ASPECT_RATIO::num) / static_cast<float>(ASPECT_RATIO::den);

constexpr auto friction = 0.0f;

constexpr auto half_pad_thickness = pad_thickness / 2.0f;
constexpr auto offset_magnitude = 0.2f;
constexpr auto x_offset = vis::vec2{half_pad_thickness, 0.0f} + offset_magnitude;
constexpr auto y_offset = vis::vec2{0.0f, half_pad_thickness};
constexpr auto half_pad_extent = vis::vec2{half_pad_thickness, half_pad_length};

namespace colors {
constexpr auto black = vis::vec4{0.0f, 0.0f, 0.0f, 1.0f};
constexpr auto red = vis::vec4{1.0f, 0.0f, 0.0f, 1.0f};
constexpr auto white = vis::vec4{1.0f, 1.0f, 1.0f, 1.0f};
} // namespace colors

} // namespace Game
