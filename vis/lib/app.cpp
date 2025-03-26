module;

#include <SDL3/SDL_main.h>

#include <SDL3/SDL.h>

export module vis:app;

import std;
import :window;

export namespace vis::app {
enum class AppResult { app_continue = SDL_APP_CONTINUE, success = SDL_APP_SUCCESS, failure = SDL_APP_FAILURE };
} // namespace vis::app

namespace {

inline SDL_AppResult to_sdl(vis::app::AppResult app_result) {
	return static_cast<SDL_AppResult>(app_result);
}

inline vis::app::AppResult from_sdl(SDL_AppResult app_result) {
	return static_cast<vis::app::AppResult>(app_result);
}

vis::win::Event from_sdl(SDL_Event& event) {
	switch (event.type) {
	case SDL_EVENT_QUIT:
		return vis::win::QuitEvent{};

	case SDL_EVENT_KEY_DOWN:
		return vis::win::KeyboardKeyDownEvent{
				.key = static_cast<vis::win::VirtualKey>(event.key.key),
				.pressed = event.key.down ? vis::win::Pressed::yes : vis::win::Pressed::no,
				.repeated = event.key.repeat ? vis::win::Repeated::yes : vis::win::Repeated::no,
		};
		break;

	case SDL_EVENT_KEY_UP:
		return vis::win::KeyboardKeyUpEvent{
				.key = static_cast<vis::win::VirtualKey>(event.key.key),
				.pressed = event.key.down ? vis::win::Pressed::yes : vis::win::Pressed::no,
				.repeated = event.key.repeat ? vis::win::Repeated::yes : vis::win::Repeated::no,
		};

	case SDL_EVENT_WINDOW_RESIZED:
		return vis::win::WindowsResized{.width = event.window.data1, .height = event.window.data2};

	default:
		return vis::win::NullEvent{};
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
export extern void on_quit(void* appstate, vis::app::AppResult result);
}

extern "C" {

SDLMAIN_DECLSPEC SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
	const auto res = ::to_sdl(::on_init(appstate, argc, argv));
	return res;
}

SDLMAIN_DECLSPEC SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	const auto res = ::to_sdl(::on_event(appstate, ::from_sdl(*event)));
	return res;
}

SDLMAIN_DECLSPEC SDL_AppResult SDL_AppIterate(void* appstate) {
	const auto res = ::to_sdl(::on_iterate(appstate));
	return res;
}

SDLMAIN_DECLSPEC void SDL_AppQuit(void* appstate, SDL_AppResult result) {
	::on_quit(appstate, ::from_sdl(result));
}
}