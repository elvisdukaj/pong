
module;

#include <cstdlib>

export module game:components;

import :events;

import std;
import vis;

export namespace Game {

struct AiComponent {
	float speed = 0.0;
};

struct BallComponent {
	vis::vec2 position;
	vis::vec2 velocity;
};

struct InputComponent {
	vis::vec2 direction{};
};

struct PlayerSpeed {
	float speed = 0.0f;
};

} // namespace Game
