
module;

#include <cstdlib>

export module game:components;

import :events;

import std;
import vis;

export namespace Game {


struct Ai {
	float speed = 0.0;
};

	struct Ball {
		vis::vec2 position;
		vis::vec2 velocity;
	};

	struct InputComponent {
		vis::vec2 direction{};
	};

	struct Player {
		float speed = 0.0f;
	};

}
