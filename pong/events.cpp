module;

#include <SDL3/SDL_events.h>

export module game:events;

import std;
import vis;

export namespace Game {

struct DefendEvent {
	DefendEvent(vis::vec2 pad_pos) : pad_pos{pad_pos} {}
	vis::vec2 pad_pos;
};

struct FollowingEvent {
	FollowingEvent(vis::vec2 pad_pos, float ball_y_pos) : pad_pos{pad_pos}, ball_y_pos{ball_y_pos} {}

	vis::vec2 pad_pos;
	float ball_y_pos;
};

struct KeyDownEvent {
	SDL_KeyboardEvent key;
};

struct KeyUpEvent {
	SDL_KeyboardEvent key;
};

} // namespace Game