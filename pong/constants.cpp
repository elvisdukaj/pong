module;

export module game:constants;

import vis;
import std;

export namespace Game {

constexpr vis::vec2 up{0.0f, 1.0f};
constexpr vis::vec2 down{0.0f, -1.0f};

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

} // namespace Game
