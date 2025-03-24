#include <SDL3/SDL_main.h>

import game;

extern "C" {

SDL_AppResult SDL_AppInit(void** appstate, [[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
	*appstate = Game::App::create();

	if (*appstate == nullptr) {
		return SDL_AppResult::SDL_APP_FAILURE;
	}

	return SDL_AppResult::SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	Game::App& app = *static_cast<Game::App*>(appstate);
	return app.process_event(event);
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	Game::App& app = *static_cast<Game::App*>(appstate);
	return app.update();
}

void SDL_AppQuit(void* appstate, SDL_AppResult) {
	delete static_cast<Game::App*>(appstate);
}

} // extern "C"
