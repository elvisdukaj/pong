
module;

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <cstdlib>
#include <print>

export module game:app;

import :events;
import :ai;
import :components;
import :constants;

import std;
import vis;

export {
	namespace Game {
	using namespace vis::literals::chrono_literals;

	class App {
	public:
		static App* create() {
			static SDL_Window* window = SDL_CreateWindow("Hello OpenGL", SCREEN_WIDTH, SCREEN_HEIGHT, screen_flags);

			if (not window) {
				throw std::runtime_error(std::format("Unable to create the window: {}", SDL_GetError()));
			}

			static auto engine = vis::engine::create(window);
			return new App{window, engine};
		}

		~App() {
			SDL_DestroyWindow(window);
		}

		[[nodiscard]] SDL_AppResult processEvent(const SDL_Event* event) noexcept {
			if (event->type == SDL_EVENT_QUIT) {
				return SDL_AppResult::SDL_APP_SUCCESS;
			}

			switch (event->type) {
			case SDL_EVENT_KEY_UP:
				dispatcher.trigger<KeyUpEvent>({event->key});
				break;

			case SDL_EVENT_KEY_DOWN: {
				dispatcher.trigger<KeyDownEvent>({event->key});
				switch (event->key.key) {
				case SDLK_ESCAPE:
					return SDL_AppResult::SDL_APP_SUCCESS;
				}
			} break;

			case SDL_EVENT_WINDOW_RESIZED:
				screen_width = event->window.data1;
				screen_height = event->window.data2;
				engine.set_viewport(0, 0, screen_width, screen_height);
				screen_proj = vis::orthogonal_matrix(screen_width, screen_height, 20.0f, 20.0f);
			}

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

		[[nodiscard]] SDL_AppResult update() noexcept {
			const auto dt = timer.elapsed();
			timer.reset();

			engine.clear();

			update_physic_system(dt);
			update_ai_system(dt);
			update_input_system(dt);
			update_ball_system(dt);
			render_system();
			engine.render(window);

			if (not is_playing) {
				std::println("You {}!", win ? "win" : "lose");
				std::println("Player {} - Computer {}", win_games, lost_games);
				reset_scene();
				is_playing = true;
			}

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

	private:
		enum class IsPlayer : bool { yes = true, no = false };

		explicit App(SDL_Window* window, vis::engine::Engine& engine) : window{window}, engine(engine) {
			initialize_video();
			initialize_game();
		}

		void initialize_game() {
			initialize_physics();
			initialize_scene();
			timer.reset();
		}

		void reset_scene() {
			entity_registry.clear();
			world = std::nullopt;

			initialize_game();
		}

		void initialize_video() {
			engine.print_info();
			engine.set_clear_color(colors::black);
			engine.set_viewport(0, 0, screen_width, screen_height);
			screen_proj = vis::orthogonal_matrix(screen_width, screen_height, world_width, world_height);
		}

		void render_system() {
			mesh_shader.bind();

			entity_registry
					.view<vis::mesh::Mesh, vis::physics::RigidBody>() //
					.each([&](const vis::mesh::Mesh& mesh, const vis::physics::RigidBody& rb) {
						const vis::mat4 model_view = rb.get_model();
						const auto model_view_projection = screen_proj.projection * model_view;
						mesh_shader.set_model_view_projection(model_view_projection);
						mesh.draw(mesh_shader);
						mesh.unbind();
					});

			// TODO: maybe a ScopedBinder<MeshBinder> _{mesh_shader} ?
			mesh_shader.unbind();
		}

		void update_input_system(vis::chrono::seconds dt) {
			entity_registry
					.view<Player, InputComponent, vis::physics::RigidBody>() //
					.each([&](const Player player_component, const InputComponent& input, vis::physics::RigidBody& rb) {
						auto transform = rb.get_transform();
						auto& pos = transform.position;

						pos += input.direction * dt * player_component.speed;
						pos.y = std::clamp(pos.y, -max_upper_bound(), max_upper_bound());

						rb.set_transform(transform);
					});
		}

		void update_ai_system(vis::chrono::seconds dt) {
			const auto& ball = entity_registry.get<Ball>(ball_entity);

			entity_registry
					.view<Ai, vis::physics::RigidBody>() //
					.each([&](Ai ai, vis::physics::RigidBody& ai_pad_rb) {
						using namespace vis::literals::chrono_literals;

						auto pad_transform = ai_pad_rb.get_transform();
						auto& pad_pos = pad_transform.position;

						auto predicted_ball_pos = ball.position + ball.velocity * dt;
						auto y_pad_ball_distance = pad_pos.y - predicted_ball_pos.y;

						if (ball.velocity.x < 0.0f) {
							ai_state_machine.process_event(FollowingEvent{pad_pos, predicted_ball_pos.y});
						} else {
							ai_state_machine.process_event(DefendEvent{pad_pos});
						}

						pad_pos += ai_context.ai_direction * dt * ai.speed;
						auto new_y_pad_ball_distance = pad_pos.y - predicted_ball_pos.y;

						if (new_y_pad_ball_distance * y_pad_ball_distance < 0.0f) {
							// clamp the y
							pad_pos.y = predicted_ball_pos.y; // avoid to run too fast
						}

						pad_pos.y = std::clamp(pad_pos.y, -max_upper_bound(), max_upper_bound());
						ai_pad_rb.set_transform(pad_transform);
					});
		}

		void update_physic_system(vis::chrono::seconds dt) {
			static auto accumulated_time = 0.0_s;
			constexpr auto fixed_time_step = 1.0_s / 30.0_s;

			accumulated_time += dt;

			while (accumulated_time >= fixed_time_step) {
				world->step(fixed_time_step, 4);
				accumulated_time -= fixed_time_step;
			}
		}

		void update_ball_system([[maybe_unused]] vis::chrono::seconds dt) {
			auto sensor_events = world->get_sensor_events();
			for (auto begin_touch_it = sensor_events.begin_begin_touch(); //
					 begin_touch_it != sensor_events.end_begin_touch();				//
					 ++begin_touch_it) {
				is_playing = false;

				auto sensor_entity = begin_touch_it->get_sensor_entity();

				win = sensor_entity == ai_sensor;
				win_games += win;
				lost_games += !win;
			}

			entity_registry
					.view<vis::physics::RigidBody, Ball>() //
					.each([&](vis::physics::RigidBody& rb, Ball& ball) {
						ball.position = rb.get_transform().position;
						ball.velocity = rb.get_linear_velocity();

						using namespace vis::literals;

						static vis::chrono::Timer acceleration_timer;
						if (acceleration_timer.elapsed() > 2.0_s) {
							const auto acceleration = vis::normalize(ball.velocity) * ball_acceleration_magnitude;
							rb.set_linear_velocity(ball.velocity + acceleration);
							acceleration_timer.reset();
						}
					});
		}

		void initialize_physics() {
			auto world_def = vis::physics::WorldDef();
			world_def.set_gravity(vis::vec2{0.0f, 0.0f * -9.81f});
			world = vis::physics::create_world(world_def);
		}

		void initialize_scene() {
			const auto half_screen_extent = screen_proj.half_world_extent;
			constexpr auto half_pad_thickness = pad_thickness / 2.0f;
			constexpr auto offset_magnitude = 0.2f;
			constexpr auto x_offset = vis::vec2{half_pad_thickness, 0.0f} + offset_magnitude;
			constexpr auto y_offset = vis::vec2{0.0f, half_pad_thickness};
			constexpr auto half_pad_extent = vis::vec2{half_pad_thickness, pad_length / 2.0f};
			const auto left_pos = vis::vec2{-half_screen_extent.x, 0.0f} + x_offset;
			const auto right_pos = vis::vec2{+half_screen_extent.x, 0.0f} - x_offset;
			const auto top_pos = vis::vec2{0.0f, +half_screen_extent.y} - y_offset;
			const auto bottom_pos = vis::vec2{0.0f, -half_screen_extent.y} + y_offset;

			const auto vertical_half_extent = vis::vec2{half_screen_extent.x, half_wall_thickness};
			const auto horizontal_half_extent = vis::vec2{half_wall_thickness, half_screen_extent.y};

			add_pad(half_pad_extent, left_pos, colors::white);
			add_player(half_pad_extent, right_pos, colors::white);

			add_ball(ball_radius, {}, {-10.0f, 10.0f}, colors::white);

			add_wall(vertical_half_extent, top_pos, colors::white);
			add_wall(vertical_half_extent, bottom_pos, colors::white);

			add_goal(horizontal_half_extent, left_pos - vis::vec2{2.0f * half_wall_thickness + offset_magnitude, 0.0f},
							 colors::black, IsPlayer::no);
			add_goal(horizontal_half_extent, right_pos + vis::vec2{2.0f * half_wall_thickness + offset_magnitude, 0.0f},
							 colors::black, IsPlayer::yes);

			dispatcher.sink<KeyDownEvent>().connect<&App::on_key_down>(this);
			dispatcher.sink<KeyUpEvent>().connect<&App::on_key_up>(this);
		}

		void on_key_down(const KeyDownEvent& event) {
			auto& input_component = entity_registry.get<InputComponent>(player);

			switch (event.key.key) {
			case SDLK_DOWN:
				input_component.direction = down;
				break;
			case SDLK_UP:
				input_component.direction = up;
				break;
			}
		}

		void on_key_up(const KeyUpEvent& event) {
			auto& input_component = entity_registry.get<InputComponent>(player);
			switch (event.key.key) {
			case SDLK_DOWN:
			case SDLK_UP:
				input_component.direction = vis::vec2{};
				break;
			}
		}

		void add_player(vis::vec2 half_extent, vis::vec2 pos, vis::vec4 color) {
			constexpr auto origin = vis::vec2{0.0f, 0.0f};
			player = entity_registry.create();
			entity_registry.emplace<Player>(player, Player{.speed = initial_player_speed});
			entity_registry.emplace<InputComponent>(player, InputComponent{});
			entity_registry.emplace<vis::mesh::Mesh>(player, vis::mesh::create_rectangle_shape(origin, half_extent, color));
			auto body_def = vis::physics::RigidBodyDef{} //
													.set_position(pos)			 //
													.set_body_type(vis::physics::BodyType::kinematic);
			auto& rigid_body = entity_registry.emplace<vis::physics::RigidBody>(player, world->create_body(body_def));

			auto wall_box = vis::physics::create_box2d(half_extent);
			vis::physics::ShapeDef wall_shape;
			rigid_body.create_shape(wall_shape, wall_box);
		}

		void add_pad(vis::vec2 half_extent, vis::vec2 pos, vis::vec4 color) {
			constexpr auto origin = vis::vec2{0.0f, 0.0f};
			auto pad = entity_registry.create();
			entity_registry.emplace<Ai>(pad, Ai{.speed = initial_ai_speed});
			entity_registry.emplace<vis::mesh::Mesh>(pad, vis::mesh::create_rectangle_shape(origin, half_extent, color));

			auto body_def = vis::physics::RigidBodyDef{}
													.set_position(pos) //
													.set_body_type(vis::physics::BodyType::kinematic);

			auto& rigid_body = entity_registry.emplace<vis::physics::RigidBody>(pad, world->create_body(body_def));

			auto wall_box = vis::physics::create_box2d(half_extent);
			auto shape = vis::physics::ShapeDef{} //
											 .set_restitution(1.0f);
			rigid_body
					.create_shape(shape, wall_box) //
					.set_entity(pad);
		}

		void add_ball(float radius, vis::vec2 pos, vis::vec2 vel, vis::vec4 color) {
			ball_entity = entity_registry.create();
			entity_registry.emplace<Ball>(ball_entity);
			entity_registry.emplace<vis::mesh::Mesh>(ball_entity, vis::mesh::create_regular_shape({}, radius, color, 10));

			auto circle = vis::physics::Circle{
					.center = {},
					.radius = radius,
			};

			auto body_def = vis::physics::RigidBodyDef{} //
													.set_position(pos)
													.set_body_type(vis::physics::BodyType::dynamic)
													.set_linear_velocity(vel)
													.set_is_bullet(true);

			auto& rigid_body = entity_registry.emplace<vis::physics::RigidBody>(ball_entity, vis::physics::RigidBody{
																																													 world->create_body(body_def),
																																											 });
			auto shape_def = vis::physics::ShapeDef{}	 //
													 .set_restitution(1.0) //
													 .set_friction(0.0f);

			rigid_body
					.create_shape(shape_def, circle) //
					.set_entity(ball_entity);
		}

		void add_wall(vis::vec2 half_extent, vis::vec2 pos, vis::vec4 color) {
			constexpr auto origin = vis::vec2{0.0f, 0.0f};
			auto wall = entity_registry.create();

			entity_registry.emplace<vis::mesh::Mesh>(wall, vis::mesh::create_rectangle_shape(origin, half_extent, color));

			auto body_def = vis::physics::RigidBodyDef{}
													.set_position(pos) //
													.set_body_type(vis::physics::BodyType::fixed);

			auto& rigid_body = entity_registry.emplace<vis::physics::RigidBody>(wall, world->create_body(body_def));

			auto wall_box = vis::physics::create_box2d(half_extent);
			auto wall_shape = vis::physics::ShapeDef{} //
														.set_friction(1.0f);

			rigid_body.create_shape(wall_shape, wall_box);
		}

		void add_goal(vis::vec2 half_extent, vis::vec2 pos, vis::vec4 color, IsPlayer is_player) {

			constexpr auto origin = vis::vec2{0.0f, 0.0f};

			auto& entity = is_player == IsPlayer::yes ? player_sensor : ai_sensor;
			entity = entity_registry.create();

			entity_registry.emplace<vis::mesh::Mesh>(entity, vis::mesh::create_rectangle_shape(origin, half_extent, color));
			auto body_def = vis::physics::RigidBodyDef{}
													.set_position(pos) //
													.set_body_type(vis::physics::BodyType::fixed);
			auto& rigid_body = entity_registry.emplace<vis::physics::RigidBody>(entity, world->create_body(body_def));
			auto wall_box = vis::physics::create_box2d(half_extent);
			auto wall_shape = vis::physics::ShapeDef{} //
														.set_is_sensor(true) //
														.set_friction(1.0f);

			// assign the
			rigid_body.create_shape(wall_shape, wall_box);
			rigid_body.set_entity(entity);
		}

		float max_upper_bound() const {
			return screen_proj.half_world_extent.y - 2 * half_wall_thickness - pad_length / 2.0f;
		}

	private:
		SDL_Window* window = nullptr;
		vis::engine::Engine& engine;

		int screen_width = SCREEN_WIDTH;
		int screen_height = SCREEN_HEIGHT;
		static constexpr SDL_WindowFlags screen_flags = SDL_WINDOW_OPENGL;

		vis::mesh::MeshShader mesh_shader{};
		vis::ecs::registry entity_registry;
		vis::ecs::dispatcher dispatcher;
		vis::ScreenProjection screen_proj;

		std::optional<vis::physics::World> world;

		vis::chrono::Timer timer;

		AiContext ai_context;
		sm<AiState> ai_state_machine{ai_context};

		bool is_playing = true;
		bool win = false;
		vis::ecs::entity ball_entity;
		vis::ecs::entity ai_sensor;
		vis::ecs::entity player_sensor;
		vis::ecs::entity player;

		int win_games = 0;
		int lost_games = 0;
	};

	} // namespace Game
}
