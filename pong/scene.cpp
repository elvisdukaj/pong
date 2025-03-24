
module;

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

	[[nodiscard]] virtual vis::win::AppResult process_event(const vis::win::Event* event) noexcept = 0;
	[[nodiscard]] virtual vis::win::AppResult update() noexcept = 0;
};

} // namespace Game
