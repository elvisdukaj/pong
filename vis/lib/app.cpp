module;

#include <SDL3/SDL.h>

export module vis:app;

import std;
import :window;

export namespace vis::app {

enum class AppResult { app_continue = SDL_APP_CONTINUE, success = SDL_APP_SUCCESS, failure = SDL_APP_FAILURE };

using OnInit = AppResult (*)(void** appstate, int argc, char** argv);
using OnEvent = AppResult (*)(void* appstate, const vis::win::Event& event);
using OnIterate = AppResult (*)(void* appstate);
using OnQuit = void (*)(void* appstate);
} // namespace vis::app

namespace {

inline SDL_AppResult to_sdl(vis::app::AppResult app_result) {
	return static_cast<SDL_AppResult>(app_result);
}

vis::win::Event from_sdl(SDL_Event& event) {
	switch (event.type) {
	case SDL_EVENT_QUIT:
		return vis::win::QuitEvent{};

	case SDL_EVENT_KEY_DOWN:
		return vis::win::KeyboardEvent{
				.type = vis::win::KeyboardEvent::key_down,
				.key = static_cast<vis::win::VirtualKey>(event.key.key),
				.pressed = event.key.down ? vis::win::Pressed::yes : vis::win::Pressed::no,
				.repeated = event.key.repeat ? vis::win::Repeated::yes : vis::win::Repeated::no,
		};
		break;

	case SDL_EVENT_KEY_UP:
		return vis::win::KeyboardEvent{
				.type = vis::win::KeyboardEvent::key_up,
				.key = static_cast<vis::win::VirtualKey>(event.key.key),
				.pressed = event.key.down ? vis::win::Pressed::yes : vis::win::Pressed::no,
				.repeated = event.key.repeat ? vis::win::Repeated::yes : vis::win::Repeated::no,
		};

	default:
		return vis::win::QuitEvent{};
	}

	return vis::win::QuitEvent{};
}

} // namespace

// The following variables are external and should be defined to the main.cpp file and shouldn't be attached to this
// module
extern "C++" {
export extern vis::app::AppResult on_init(void** appstate, int argc, char** argv);
export extern vis::app::AppResult on_event(void* appstate, const vis::win::Event& event);
export extern vis::app::AppResult on_iterate(void* appstate);
export extern void on_quit(void* appstate);
}

extern "C" {

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
	return ::to_sdl(::on_init(appstate, argc, argv));
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	return ::to_sdl(::on_event(appstate, ::from_sdl(*event)));
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	return ::to_sdl(::on_iterate(appstate));
}

void SDL_AppQuit(void* appstate) {
	::on_quit(appstate);
}
}