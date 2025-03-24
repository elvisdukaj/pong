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
	case SDL_EVENT_KEY_UP:
		return vis::win::KeyboardEvent{
				.type = event.type == SDL_EVENT_KEY_DOWN ? vis::win::KeyboardEvent::key_down : vis::win::KeyboardEvent::key_up,
				.key = static_cast<vis::win::VirtualKey>(event.key.key),
				.pressed = event.key.down ? vis::win::Pressed::yes : vis::win::Pressed::no,
				.repeated = event.key.repeat ? vis::win::Repeated::yes : vis::win::Repeated::no,
		};
	}

	return vis::win::QuitEvent{};
}

} // namespace

// The following variables are external and should be defined to the main.cpp file and shouldn't be attached to this
// module
extern "C++" {
export extern vis::app::OnInit on_init;
export extern vis::app::OnEvent on_event;
export extern vis::app::OnIterate on_iterate;
export extern vis::app::OnQuit on_quit;
}

extern "C" {

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
	auto result = ::on_init(appstate, argc, argv);
	return ::to_sdl(result);
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	auto result = ::on_event(appstate, ::from_sdl(*event));
	return ::to_sdl(result);
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	auto result = ::on_iterate(appstate);
	return ::to_sdl(result);
}

void SDL_AppQuit(void* appstate) {
	::on_quit(appstate);
}
}