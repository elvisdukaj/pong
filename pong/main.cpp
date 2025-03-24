#include <SDL3/SDL_main.h>

import game;
import vis;

vis::app::OnInit on_init = [](void** appstate, [[maybe_unused]] int argc,
															[[maybe_unused]] char** argv) -> vis::app::AppResult {
	*appstate = Game::App::create();

	if (*appstate == nullptr)
		return vis::app::AppResult::failure;

	return vis::app::AppResult::app_continue;
};

vis::app::OnEvent on_event = [](void* appstate, const vis::win::Event& event) -> vis::app::AppResult {
	Game::App& app = *static_cast<Game::App*>(appstate);
	return app.process_event(event);
};

vis::app::OnIterate on_iterate = [](void* appstate) -> vis::app::AppResult {
	Game::App& app = *static_cast<Game::App*>(appstate);
	return app.update();
};

vis::app::OnQuit on_quit = [](void* appstate) -> void { delete static_cast<Game::App*>(appstate); };