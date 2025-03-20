
module;

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <boost/sml.hpp>

#include <cstdlib>
#include <print>

export module Game;

import std;
import vis;

export {
	namespace Game {

	using namespace vis::literals::chrono_literals;

	struct Ball {
		vis::vec2 position;
		vis::vec2 velocity;
	};

	struct Ai {
		float speed = 0.0;
	};

	struct DefendEvent {
		DefendEvent(vis::vec2 pad_pos) : pad_pos{pad_pos} {}

		vis::vec2 pad_pos;
	};

	struct FollowingEvent {
		FollowingEvent(vis::vec2 pad_pos, float ball_y_pos) : pad_pos{pad_pos}, ball_y_pos{ball_y_pos} {}

		vis::vec2 pad_pos;
		float ball_y_pos;
	};

	struct GameOver {
		bool win = false;
	};

	struct KeyDownEvent {
		SDL_KeyboardEvent key;
	};

	struct KeyUpEvent {
		SDL_KeyboardEvent key;
	};

	struct AiContext {
		vis::vec2 ai_direction;
	};

	constexpr vis::vec2 up{0.0f, 1.0f};
	constexpr vis::vec2 down{0.0f, -1.0f};

	auto defend = [](const DefendEvent& event, AiContext& context) {
		const auto& [pad_pos] = event;
		if (std::abs(pad_pos.y) < 0.3) {
			context.ai_direction = vis::vec2{0.0f, 0.0f};
			return;
		}

		context.ai_direction = (pad_pos.y < 0.0f) ? up : down;
	};

	auto follow = [](const FollowingEvent& event, AiContext& context) {
		const auto& [pad_pos, ball_y_pos] = event;
		context.ai_direction = (ball_y_pos > pad_pos.y) ? up : down;
	};

	struct AiState {
		auto operator()() const {
			using namespace boost::sml;

			return make_transition_table(
					// clang-format off
				* "defend"_s	+ event<FollowingEvent>		/ follow						= "follow"_s,
					"defend"_s	+ event<DefendEvent>			/ defend						= "defend"_s,
					"defend"_s	+ event<GameOver>										 					= X,
					"follow"_s	+ event<FollowingEvent>		/ follow			    	= "follow"_s,
					"follow"_s	+ event<DefendEvent>			/ defend						= "defend"_s,
					"follow"_s	+ event<GameOver>															= X
					// clang-format on
			);
		}
	};

	struct InputComponent {
		vis::vec2 direction{};
	};

	struct Player {
		float speed = 0.0f;
	};

	constexpr int SCREEN_HEIGHT = 600;
	constexpr std::ratio<4, 3> ASPECT_RATIO;
	constexpr int SCREEN_WIDTH = 800; // SCREEN_HEIGHT * ASPECT_RATIO.num / ASPECT_RATIO.den;

	class App {
	public:
		static App* create() {
			srand(static_cast<unsigned int>(SDL_GetTicks()));
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

			update_ai_system(dt);
			update_input_system(dt);
			update_physic_system(dt);
			update_ball_system(dt);
			render_system();
			engine.render(window);

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

	private:
		explicit App(SDL_Window* window, vis::engine::Engine& engine) : window{window}, engine(engine) {
			initialize_video();
			initialize_physics();
			initialize_scene();

			timer.reset();
		}

		void initialize_video() {
			engine.print_info();
			engine.set_clear_color(vis::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			engine.set_viewport(0, 0, screen_width, screen_height);
			screen_proj = vis::orthogonal_matrix(screen_width, screen_height, 20.0f, 20.0f);
		}

		void render_system() {
			const auto view = entity_registry.view<vis::mesh::Mesh, vis::physics::RigidBody>();
			mesh_shader.bind();

			view.each([&](const vis::mesh::Mesh& mesh, const vis::physics::RigidBody& rb) {
				const vis::mat4 model_view = rb.get_transform().get_model();
				const auto model_view_projection = screen_proj.projection * model_view;
				mesh_shader.set_model_view_projection(model_view_projection);
				mesh.draw(mesh_shader);
				mesh.unbind();
			});

			// TODO: maybe a ScopedBinder<MeshBinder> _{mesh_shader} ?
			mesh_shader.unbind();
		}

		void update_input_system(vis::chrono::seconds dt) {
			const auto view = entity_registry.view<Player, InputComponent, vis::physics::RigidBody>();
			view.each([&](const Player player, const InputComponent& input, vis::physics::RigidBody& rb) {
				auto transform = rb.get_transform();
				auto& pos = transform.position;

				pos += input.direction * dt * player.speed;
				pos.y = std::clamp(pos.y, -max_upper_bound(), max_upper_bound());

				rb.set_transform(transform);
			});
		}

		void update_ai_system(vis::chrono::seconds dt) {
			entity_registry
					.view<Ai, vis::physics::RigidBody>() //
					.each([&](Ai ai, vis::physics::RigidBody& ai_pad_rb) {
						entity_registry
								.view<Ball>() //
								.each([&](Ball ball) {
									using namespace boost::sml;
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
										pad_pos.y = predicted_ball_pos.y; // avoid to run too fast
									}

									pad_pos.y = std::clamp(pad_pos.y, -max_upper_bound(), max_upper_bound());
									ai_pad_rb.set_transform(pad_transform);
								});
					});
		}

		void update_physic_system(vis::chrono::seconds dt) const {
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
				auto sensor = begin_touch_it->sensor_shape_def();
				auto body = begin_touch_it->visitor_shape_def();
				std::println("visitor {} collided with {}", static_cast<uint64_t>(body->get_entity()),
										 static_cast<uint64_t>(sensor->get_entity()));
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

						if (ball.position.x < -screen_proj.half_world_extent.x) {
							std::println("Player won");
							ai_state_machine.process_event(GameOver{true});
						}
						if (ball.position.x > screen_proj.half_world_extent.x) {
							std::println("AI won");
							ai_state_machine.process_event(GameOver{false});
						}
					});
		}

		void initialize_physics() {
			auto world_def = vis::physics::WorldDef();
			world_def.set_gravity(vis::vec2{0.0f, 0.0f * -9.81f});
			world = vis::physics::create_world(world_def);
		}

		void initialize_scene() {
			constexpr auto white = vis::vec4{1.0f, 1.0f, 1.0f, 1.0f};
			constexpr auto black = vis::vec4{0.0f, 0.0f, 1.0f, 1.0f};

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

			add_pad(half_pad_extent, left_pos, white);
			add_player(half_pad_extent, right_pos, white);

			add_ball(ball_radius, {}, {-10.0f, 10.0f}, white);

			add_wall(vertical_half_extent, top_pos, white);
			add_wall(vertical_half_extent, bottom_pos, white);

			// this should be invisible
			add_goal(horizontal_half_extent, left_pos - vis::vec2{2.0f * half_wall_thickness + offset_magnitude, 0.0f},
							 black);
			add_goal(horizontal_half_extent, right_pos + vis::vec2{2.0f * half_wall_thickness + offset_magnitude, 0.0f},
							 black);

			dispatcher.sink<KeyDownEvent>().connect<&App::on_key_down>(this);
			dispatcher.sink<KeyUpEvent>().connect<&App::on_key_up>(this);
		}

		void on_key_down(const KeyDownEvent& event) {
			switch (event.key.key) {
			case SDLK_DOWN:
				entity_registry
						.view<InputComponent>() //
						.each([](InputComponent& input) { input.direction.y = -1.0f; });
				break;
			case SDLK_UP:
				entity_registry
						.view<InputComponent>() //
						.each([](InputComponent& input) { input.direction.y = 1.0f; });
				break;
			}
		}

		void on_key_up(const KeyUpEvent& event) {
			switch (event.key.key) {
			case SDLK_DOWN:
				entity_registry
						.view<InputComponent>() //
						.each([](InputComponent& input) { input.direction.y = .0f; });
				break;
			case SDLK_UP:
				entity_registry
						.view<InputComponent>() //
						.each([](InputComponent& input) { input.direction.y = .0f; });
				break;

			default:
				break;
			}
		}

		void add_player(vis::vec2 half_extent, vis::vec2 pos, vis::vec4 color) {
			constexpr auto origin = vis::vec2{0.0f, 0.0f};
			auto pad = entity_registry.create();
			entity_registry.emplace<Player>(pad, Player{.speed = initial_player_speed});
			entity_registry.emplace<InputComponent>(pad, InputComponent{});
			entity_registry.emplace<vis::mesh::Mesh>(pad, vis::mesh::create_rectangle_shape(origin, half_extent, color));
			auto& transform = entity_registry.emplace<vis::physics::Transform>(pad, vis::physics::Transform{
																																									.position = pos,
																																							});
			vis::physics::RigidBodyDef body_def;
			body_def															//
					.set_position(transform.position) //
					.set_body_type(vis::physics::BodyType::kinematic);
			auto& rigid_body = entity_registry.emplace<vis::physics::RigidBody>(pad, world->create_body(body_def));

			auto wall_box = vis::physics::create_box2d(half_extent);
			vis::physics::ShapeDef wall_shape;
			rigid_body.create_shape(wall_shape, wall_box);
		}

		void add_pad(vis::vec2 half_extent, vis::vec2 pos, vis::vec4 color) {
			constexpr auto origin = vis::vec2{0.0f, 0.0f};
			auto pad = entity_registry.create();
			entity_registry.emplace<Ai>(pad, Ai{.speed = initial_ai_speed});
			entity_registry.emplace<vis::mesh::Mesh>(pad, vis::mesh::create_rectangle_shape(origin, half_extent, color));

			vis::physics::RigidBodyDef body_def;
			body_def							 //
					.set_position(pos) //
					.set_body_type(vis::physics::BodyType::kinematic);
			auto& rigid_body = entity_registry.emplace<vis::physics::RigidBody>(pad, world->create_body(body_def));

			auto wall_box = vis::physics::create_box2d(half_extent);
			auto shape = vis::physics::ShapeDef{} //
											 .set_restitution(1.0f);
			rigid_body.create_shape(shape, wall_box);
		}

		void add_ball(float radius, vis::vec2 pos, vis::vec2 vel, vis::vec4 color) {
			auto ball = entity_registry.create();

			entity_registry.emplace<Ball>(ball);
			entity_registry.emplace<vis::mesh::Mesh>(ball, vis::mesh::create_regular_shape({}, radius, color, 10));

			auto circle = vis::physics::Circle{
					.center = {},
					.radius = radius,
			};

			auto body_def = vis::physics::RigidBodyDef{} //
													.set_position(pos)
													.set_body_type(vis::physics::BodyType::dynamic)
													.set_linear_velocity(vel)
													.set_is_bullet(true);

			auto& rigid_body = entity_registry.emplace<vis::physics::RigidBody>(ball, vis::physics::RigidBody{
																																										world->create_body(body_def),
																																								});
			auto shape_def = vis::physics::ShapeDef{}	 //
													 .set_restitution(1.0) //
													 .set_friction(0.0f);

			rigid_body
					.create_shape(shape_def, circle) //
					.set_entity(ball);

			std::println("ball entity has id {}", static_cast<uint64_t>(ball));
		}

		void add_wall(vis::vec2 half_extent, vis::vec2 pos, vis::vec4 color) {
			constexpr auto origin = vis::vec2{0.0f, 0.0f};
			auto wall = entity_registry.create();
			entity_registry.emplace<vis::mesh::Mesh>(wall, vis::mesh::create_rectangle_shape(origin, half_extent, color));
			auto body_def = vis::physics::RigidBodyDef{};
			body_def
					.set_position(pos) //
					.set_body_type(vis::physics::BodyType::fixed);
			auto& rigid_body = entity_registry.emplace<vis::physics::RigidBody>(wall, world->create_body(body_def));

			auto wall_box = vis::physics::create_box2d(half_extent);
			auto wall_shape = vis::physics::ShapeDef{} //
														.set_friction(1.0f);

			rigid_body.create_shape(wall_shape, wall_box);
		}

		void add_goal(vis::vec2 half_extent, vis::vec2 pos, vis::vec4 color) {
			constexpr auto origin = vis::vec2{0.0f, 0.0f};
			auto wall = entity_registry.create();
			entity_registry.emplace<vis::mesh::Mesh>(wall, vis::mesh::create_rectangle_shape(origin, half_extent, color));
			auto body_def = vis::physics::RigidBodyDef{};
			body_def
					.set_position(pos) //
					.set_body_type(vis::physics::BodyType::fixed);
			auto& rigid_body = entity_registry.emplace<vis::physics::RigidBody>(wall, world->create_body(body_def));
			auto wall_box = vis::physics::create_box2d(half_extent);
			auto wall_shape = vis::physics::ShapeDef{} //
														.set_is_sensor(true) //
														.set_friction(1.0f);

			// assign the
			rigid_body.create_shape(wall_shape, wall_box);
			rigid_body.set_entity(wall);
			std::println("goal entity has id {}", static_cast<uint64_t>(wall));
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

		static constexpr auto pad_thickness = 0.6f;
		static constexpr auto pad_length = 3.0f;
		static constexpr auto ball_radius = 0.4f;
		static constexpr auto half_wall_thickness = 0.3f;

		static constexpr auto initial_player_speed = 20.0f;
		static constexpr auto initial_ai_speed = 20.0f;

		static constexpr auto ball_acceleration_magnitude = 2.0f;

		vis::chrono::Timer timer;

		AiContext ai_context;
		boost::ext::sml::sm<AiState> ai_state_machine{ai_context};
	};

	} // namespace Game
}
