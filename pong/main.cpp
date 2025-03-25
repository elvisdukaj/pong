import game;
import vis;

auto on_init(void** appstate, [[maybe_unused]] int argc, [[maybe_unused]] char** argv) -> vis::app::AppResult {
	*appstate = Game::App::create();

	if (*appstate == nullptr)
		return vis::app::AppResult::failure;

	return vis::app::AppResult::app_continue;
}

auto on_event(void* appstate, const vis::win::Event& event) -> vis::app::AppResult {
	Game::App& app = *static_cast<Game::App*>(appstate);
	return app.process_event(event);
}

auto on_iterate(void* appstate) -> vis::app::AppResult {
	Game::App& app = *static_cast<Game::App*>(appstate);
	return app.update();
}

auto on_quit(void* appstate, [[maybe_unused]] vis::app::AppResult result) -> void {
	delete static_cast<Game::App*>(appstate);
}