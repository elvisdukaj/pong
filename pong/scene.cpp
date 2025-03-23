
module;

#include <SDL3/SDL.h>

export module game:scene;

import :events;
import :components;
import :constants;

import std;
import vis;

export namespace Game {

class Scene {
public:
	virtual ~Scene() = default;

	[[nodiscard]] virtual SDL_AppResult processEvent(const SDL_Event* event) noexcept = 0;
	[[nodiscard]] virtual SDL_AppResult update() noexcept = 0;
};

} // namespace Game
