
module;

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <cstdlib>
#include <print>

export module game:pong_scene;

import :events;
import :components;
import :constants;
import :scene;

import std;
import vis;

export {
	namespace Game {
	using namespace vis::literals::chrono_literals;

	class PongScene : public Scene {
	public:
		explicit PongScene(vis::opengl::OpenGLRenderer& renderer) : renderer(renderer) {
			initialize_video();
			initialize_game();
		}

		[[nodiscard]] SDL_AppResult processEvent(const SDL_Event* event) noexcept override {
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

				case SDLK_P:
					is_pausing = !is_pausing;
				}
			} break;

			case SDL_EVENT_WINDOW_RESIZED:
				screen_width = event->window.data1;
				screen_height = event->window.data2;
				renderer.set_viewport(0, 0, screen_width, screen_height);
				screen_proj = vis::orthogonal_matrix(screen_width, screen_height, 20.0f, 20.0f);
			}

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

		[[nodiscard]] SDL_AppResult update() noexcept override {
			static vis::chrono::Timer game_timer;
			const auto dt = timer.elapsed();
			timer.reset();

			renderer.clear();

			if (not is_pausing) {
				update_physic_system(dt);
				update_ai_system(dt);
				update_input_system(dt);
				update_ball_system(game_timer.elapsed());
				update_game_logic();
			}
			render_system();
			renderer.render();

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
			std::println("{}", renderer.show_info());
			renderer.set_clear_color(colors::black);
			renderer.set_viewport(0, 0, screen_width, screen_height);
			screen_proj = vis::orthogonal_matrix(screen_width, screen_height, world_width, world_height);
		}

		void render_system() {
			mesh_shader.bind();

			entity_registry
					.view<vis::mesh::Mesh, vis::physics::RigidBody>() //
					.each([&](const vis::mesh::Mesh& mesh, const vis::physics::RigidBody& rb) {
						mesh.bind();
						const vis::mat4 model_view = rb.get_model();
						const auto model_view_projection = screen_proj.projection * model_view;
						mesh_shader.set_model_view_projection(model_view_projection);
						mesh.draw(mesh_shader);
						mesh.unbind();
					});

			mesh_shader.unbind();
		}

		void update_input_system(vis::chrono::seconds dt) {
			entity_registry
					.view<PlayerSpeed, InputComponent, vis::physics::RigidBody>() //
					.each([&](const PlayerSpeed player_component, const InputComponent& input, vis::physics::RigidBody& rb) {
						auto transform = rb.get_transform();
						auto& pos = transform.position;

						pos += input.direction * dt * player_component.speed;
						pos.y = std::clamp(pos.y, -max_upper_bound(), max_upper_bound());

						rb.set_transform(transform);
					});
		}

		void update_ai_system(vis::chrono::seconds dt) {
			const BallComponent& ball = entity_registry.get<BallComponent>(ball_entity);

			entity_registry
					.view<AiComponent, vis::physics::RigidBody>() //
					.each([&](AiComponent ai, vis::physics::RigidBody& ai_pad_rb) {
						using namespace vis::literals::chrono_literals;

						auto pad_transform = ai_pad_rb.get_transform();
						auto& pad_pos = pad_transform.position;

						const auto y_pad_ball_distance = pad_pos.y - ball.position.y;
						const auto direction = (ball.position.y > pad_pos.y) ? up : down;

						pad_pos += direction * dt * ai.speed;

						auto new_y_pad_ball_distance = pad_pos.y - ball.position.y;

						if (std::signbit(new_y_pad_ball_distance) != std::signbit(y_pad_ball_distance)) {
							// clamp the y
							pad_pos.y = ball.position.y; // avoid to run too fast
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

		void update_ball_system(vis::chrono::milliseconds t) {
			auto contacts = world->get_hit_events();
			for (auto& contact : contacts) {
				auto entity_a = contact.entity_a();
				auto entity_b = contact.entity_b();

				bool is_ball = entity_a == ball_entity || entity_b == ball_entity;
				bool is_pad = entity_a == ai_entity || entity_b == ai_entity;
				bool is_player = entity_a == player_entity || entity_b == player_entity;

				if (not is_ball)
					return;

				if (!(is_player or is_pad))
					return;

				// ball-pad collision

				vis::physics::RigidBody& ball_rb = entity_registry.get<vis::physics::RigidBody>(ball_entity);
				auto ball_vel = vis::normalize(ball_rb.get_linear_velocity());

				// const auto& norm = is_player ? right_norm : left_norm;
				const auto norm = contact.normal();
				auto old_direction = vis::reflect(ball_vel, norm);
				auto new_direction = vis::get_random_direction(old_direction, ball_angle_min, ball_angle_max);
				auto new_vel_mag = vis::get_random(ball_vel_min_speed, ball_vel_max_speed);

				std::println("[{}] normal: {}, old_dir = {}, new_dire: {}", t, norm, old_direction, new_direction);

				ball_rb.set_linear_velocity(new_direction * new_vel_mag);
			}

			entity_registry
					.view<vis::physics::RigidBody, BallComponent>() //
					.each([&](vis::physics::RigidBody& rb, BallComponent& ball) {
						ball.position = rb.get_transform().position;
						ball.velocity = rb.get_linear_velocity();
					});
		}

		void update_game_logic() {
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
		}

		void initialize_physics() {
			auto world_def = vis::physics::WorldDef();
			world_def.set_gravity(vis::vec2{0.0f, 0.0f * -9.81f});
			world = vis::physics::create_world(world_def);
		}

		void initialize_scene() {
			const auto half_screen_extent = screen_proj.half_world_extent;
			const auto left_pos = vis::vec2{-half_screen_extent.x, 0.0f} + x_offset;
			const auto right_pos = vis::vec2{+half_screen_extent.x, 0.0f} - x_offset;
			const auto top_pos = vis::vec2{0.0f, +half_screen_extent.y} - y_offset;
			const auto bottom_pos = vis::vec2{0.0f, -half_screen_extent.y} + y_offset;

			const auto vertical_half_extent = vis::vec2{half_screen_extent.x, half_wall_thickness};
			const auto horizontal_half_extent = vis::vec2{half_wall_thickness, half_screen_extent.y};

			add_pad(half_pad_extent, left_pos, colors::white);
			add_player(half_pad_extent, right_pos, colors::white);
			add_ball(ball_radius, origin, colors::white);
			add_wall(vertical_half_extent, top_pos, colors::white);
			add_wall(vertical_half_extent, bottom_pos, colors::white);

			add_goal(horizontal_half_extent, left_pos - vis::vec2{wall_thickness + offset_magnitude, 0.0f}, colors::black,
							 IsPlayer::no);
			add_goal(horizontal_half_extent, right_pos + vis::vec2{wall_thickness + offset_magnitude, 0.0f}, colors::black,
							 IsPlayer::yes);

			dispatcher.sink<KeyDownEvent>().connect<&PongScene::on_key_down>(this);
			dispatcher.sink<KeyUpEvent>().connect<&PongScene::on_key_up>(this);
		}

		void on_key_down(const KeyDownEvent& event) {
			auto& input_component = entity_registry.get<InputComponent>(player_entity);

			switch (event.key.key) {
			case SDLK_DOWN:
			case SDLK_RIGHT:
				input_component.direction = down;
				break;
			case SDLK_UP:
			case SDLK_LEFT:
				input_component.direction = up;
				break;
			}
		}

		void on_key_up(const KeyUpEvent& event) {
			auto& input_component = entity_registry.get<InputComponent>(player_entity);
			switch (event.key.key) {
			case SDLK_DOWN:
			case SDLK_UP:
			case SDLK_RIGHT:
			case SDLK_LEFT:
				input_component.direction = vis::vec2{};
				break;
			}
		}

		void add_player(vis::vec2 half_extent, vis::vec2 pos, vis::vec4 color) {
			player_entity = entity_registry.create();
			entity_registry.emplace<PlayerSpeed>(player_entity, PlayerSpeed{.speed = initial_player_speed});
			entity_registry.emplace<InputComponent>(player_entity, InputComponent{});
			entity_registry.emplace<vis::mesh::Mesh>(player_entity,
																							 vis::mesh::create_rectangle_shape(origin, half_extent, color));

			auto body_def = vis::physics::RigidBodyDef{} //
													.set_position(pos)			 //
													.set_body_type(vis::physics::BodyType::kinematic);
			auto& rigid_body =
					entity_registry.emplace<vis::physics::RigidBody>(player_entity, world->create_body(body_def, player_entity));

			auto wall_box = vis::physics::create_box2d(half_extent);
			auto wall_shape = vis::physics::ShapeDef{} //
														.set_restitution(1.0f)
														.set_friction(friction);
			rigid_body.create_shape(wall_shape, wall_box);
		}

		void add_pad(vis::vec2 half_extent, vis::vec2 pos, vis::vec4 color) {
			ai_entity = entity_registry.create();
			entity_registry.emplace<AiComponent>(ai_entity, AiComponent{.speed = initial_ai_speed});
			entity_registry.emplace<vis::mesh::Mesh>(ai_entity,
																							 vis::mesh::create_rectangle_shape(origin, half_extent, color));

			auto body_def = vis::physics::RigidBodyDef{}
													.set_position(pos) //
													.set_body_type(vis::physics::BodyType::kinematic);

			auto& rigid_body =
					entity_registry.emplace<vis::physics::RigidBody>(ai_entity, world->create_body(body_def, ai_entity));

			auto wall_box = vis::physics::create_box2d(half_extent);
			auto shape = vis::physics::ShapeDef{} //
											 .set_restitution(1.0f)
											 .set_friction(friction);
			rigid_body.create_shape(shape, wall_box);
		}

		void add_ball(float radius, vis::vec2 pos, vis::vec4 color) {
			const auto vel_mag = vis::get_random(ball_vel_min_speed, ball_vel_max_speed);
			const auto direction = vis::get_random_direction(vis::vec2{-1.0f, 0.0f}, ball_angle_min, ball_angle_max);
			const auto vel = direction * vel_mag;

			ball_entity = entity_registry.create();
			entity_registry.emplace<BallComponent>(ball_entity);

			entity_registry.emplace<vis::mesh::Mesh>(ball_entity, vis::mesh::create_regular_shape(origin, radius, color, 10));

			auto circle = vis::physics::Circle{
					.center = {},
					.radius = radius,
			};

			auto body_def = vis::physics::RigidBodyDef{} //
													.set_position(pos)
													.set_body_type(vis::physics::BodyType::dynamic)
													.set_linear_velocity(vel)
													.set_fixed_rotation(false)
													.set_is_bullet(true);

			auto& rigid_body =
					entity_registry.emplace<vis::physics::RigidBody>(ball_entity, vis::physics::RigidBody{
																																						world->create_body(body_def, ball_entity),
																																				});
			auto shape_def = vis::physics::ShapeDef{} //
													 .set_restitution(1.0)
													 .set_friction(friction)
													 .enable_hit_events(true);
			rigid_body.create_shape(shape_def, circle);
		}

		void add_wall(vis::vec2 half_extent, vis::vec2 pos, vis::vec4 color) {
			auto wall = entity_registry.create();

			entity_registry.emplace<vis::mesh::Mesh>(wall, vis::mesh::create_rectangle_shape(origin, half_extent, color));

			auto body_def = vis::physics::RigidBodyDef{}
													.set_position(pos) //
													.set_body_type(vis::physics::BodyType::fixed);

			auto& rigid_body = entity_registry.emplace<vis::physics::RigidBody>(wall, world->create_body(body_def, wall));

			auto wall_box = vis::physics::create_box2d(half_extent);
			auto wall_shape = vis::physics::ShapeDef{} //
														.set_restitution(1.0f)
														.set_friction(friction);
			rigid_body.create_shape(wall_shape, wall_box);
		}

		void add_goal(vis::vec2 half_extent, vis::vec2 pos, vis::vec4 color, IsPlayer is_player) {
			auto& entity = is_player == IsPlayer::yes ? player_sensor : ai_sensor;
			entity = entity_registry.create();

			entity_registry.emplace<vis::mesh::Mesh>(entity, vis::mesh::create_rectangle_shape(origin, half_extent, color));
			auto body_def = vis::physics::RigidBodyDef{}
													.set_position(pos) //
													.set_body_type(vis::physics::BodyType::fixed);
			auto& rigid_body = entity_registry.emplace<vis::physics::RigidBody>(entity, world->create_body(body_def, entity));
			auto wall_box = vis::physics::create_box2d(half_extent);
			auto wall_shape = vis::physics::ShapeDef{} //
														.set_is_sensor(true);
			rigid_body.create_shape(wall_shape, wall_box);
		}

		float max_upper_bound() const {
			return screen_proj.half_world_extent.y - 2 * half_wall_thickness - pad_length / 2.0f;
		}

	private:
		vis::opengl::OpenGLRenderer& renderer;

		int screen_width = SCREEN_WIDTH;
		int screen_height = SCREEN_HEIGHT;
		static constexpr SDL_WindowFlags screen_flags = SDL_WINDOW_OPENGL;

		vis::mesh::MeshShader mesh_shader{};
		vis::ecs::registry entity_registry;
		vis::ecs::dispatcher dispatcher;
		vis::ScreenProjection screen_proj;

		std::optional<vis::physics::World> world;

		vis::chrono::Timer timer;

		bool is_playing = true;
		bool win = false;
		bool is_pausing = false;
		vis::ecs::entity ai_sensor;
		vis::ecs::entity player_sensor;
		vis::ecs::entity ball_entity;
		vis::ecs::entity player_entity;
		vis::ecs::entity ai_entity;

		int win_games = 0;
		int lost_games = 0;
	};

	} // namespace Game
}
