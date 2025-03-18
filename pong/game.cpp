
module;

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <cstdlib>

export module Game;

import std;
import vis;

export {
	namespace Game {

	struct Ball {
	private:
		int unused;
	};
	struct Player {};
	struct Ai {
	private:
		int unused;
	};

	struct InputComponent {
		vis::vec2 direction{};
	};

	constexpr int SCREEN_HEIGHT = 600;
	constexpr std::ratio<4, 3> ASPECT_RATIO;
	constexpr int SCREEN_WIDTH = 800; // SCREEN_HEIGHT * ASPECT_RATIO.num / ASPECT_RATIO.den;

	class App {
	public:
		static App* create() {
			srand(SDL_GetTicks());
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
				switch (event->key.key) {
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
				}
				break;
			case SDL_EVENT_KEY_DOWN: {
				switch (event->key.key) {
				case SDLK_ESCAPE:
					return SDL_AppResult::SDL_APP_SUCCESS;

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

				default:
					break;
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

		static auto now() -> float {
			return static_cast<float>(SDL_GetTicks()) / 1000.0f;
		}

		[[nodiscard]] SDL_AppResult update() noexcept {
			static float previous_time = SDL_GetTicks() / 1000.0f;
			static float ai_time = SDL_GetTicks() / 1000.0f;

			engine.clear();

			const auto t = SDL_GetTicks() / 1000.0f;
			const auto dt = t - previous_time;
			static uint64_t frame_count = 0;

			// if (ai_time > .3f) {
			update_ai_system(t, dt);
			// ai_time = 0.0f;
			// }
			update_input_system(t, dt);
			update_physic_system(t, dt);

			render_system();

			engine.render(window);

			previous_time = t;
			ai_time += dt;

			return SDL_AppResult::SDL_APP_CONTINUE;
		}

	private:
		explicit App(SDL_Window* window, vis::engine::Engine& engine)
				: window{window}, engine(engine), program{std::nullopt} {
			initialize_video();
			initialize_physics();
			initialize_scene();
		}

		void initialize_video() {
			engine.print_info();
			engine.set_clear_color(vis::vec4(0.0f, 0.0f, 0.0f, 1.0f));
			engine.set_viewport(0, 0, screen_width, screen_height);
			screen_proj = vis::orthogonal_matrix(screen_width, screen_height, 20.0f, 20.0f);

			// TODO: load this from file or even better, build using SPRIV during compilation time
			program = vis::opengl::ProgramBuilder{}
										.add_shader(vis::opengl::Shader::create(vis::opengl::ShaderType::vertex, R"(
#version 410 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 col;

uniform mat4 model_view_projection;

out vec4 vertex_color;

void main()
{
    gl_Position = model_view_projection * vec4(pos.xy, 0.0f, 1.0f);
		vertex_color = col;
}
)"))
										.add_shader(vis::opengl::Shader::create(vis::opengl::ShaderType::fragment, R"(
#version 410 core

in vec4 vertex_color;
out vec4 fragment_color;

void main()
{
//    FragColor = vec4(0.0f, 0.5f, 0.2f, 1.0f);
		fragment_color = vertex_color;
}
)"))
										.build();

			if (not program) {
				throw std::runtime_error{"Unable to initialize the video"};
			}
		}

		void render_system() {
			const auto view = entity_registry.view<vis::mesh::Mesh, vis::physics::RigidBody>();
			program->use();

			view.each([&](const vis::mesh::Mesh& mesh, const vis::physics::RigidBody& rb) {
				const vis::mat4 model_view = rb.get_transform().get_model();
				const auto model_view_projection = screen_proj.projection * model_view;
				program->set_uniform("model_view_projection", model_view_projection);
				mesh.draw(*program);
				mesh.unbind();
			});
		}

		void update_input_system(float t, float dt) {
			const auto view = entity_registry.view<InputComponent, vis::physics::RigidBody>();
			view.each([&](const InputComponent& input, vis::physics::RigidBody& rb) {
				auto transform = rb.get_transform();
				auto& pos = transform.position;

				pos += input.direction * dt * 15.0f; // TODO: set a variable here
				pos.y = std::clamp(pos.y, -max_upper_bound(), max_upper_bound());

				rb.set_transform(transform);
			});
		}

		void update_ai_system(float t, float dt) {
			entity_registry
					.view<Ai, vis::physics::RigidBody>() //
					.each([&](Ai&, vis::physics::RigidBody& ai_pad_rb) {
						entity_registry
								.view<Ball, vis::physics::RigidBody>() //
								.each([&](Ball, const vis::physics::RigidBody& ball_rigid_body) {
									auto pad_transform = ai_pad_rb.get_transform();
									auto ball_vel = ball_rigid_body.get_linear_velocity();

									auto& pad_pos = pad_transform.position;
									auto ball_pos = ball_rigid_body.get_transform().position;
									vis::vec2 direction;

									auto y_ball_pos = vis::vec2{0.0f, ball_pos.y};
									auto y_ai_pos = vis::vec2{0.0f, pad_pos.y};

									if (ball_vel.x > 0.0) {
										direction = vis::normalize(vis::vec2{pad_pos.x, 0.0f} - pad_pos);
									} else {

										float time_to_paddle =
												std::abs(pad_pos.x - ball_pos.x) / std::abs(ball_rigid_body.get_linear_velocity().x);
										vis::vec2 predicted_pos{pad_pos.x, ball_pos.y + ball_vel.y * time_to_paddle};
										direction = vis::normalize(predicted_pos - pad_pos);
									}
									// ai_pad_rb.set_linear_velocity(direction * 9.5f);
									pad_pos += direction * dt * 10.0f; // TODO: set a variable here
									pad_pos.y = std::clamp(pad_pos.y, -max_upper_bound(), max_upper_bound());
									ai_pad_rb.set_transform(pad_transform);
								});
					});
		}

		void update_physic_system(float t, [[maybe_unused]] float dt) {
			static float accumulated_time = 0.0f;
			static constexpr float fixed_time_step = 1 / 30.0f;

			accumulated_time += dt;

			while (accumulated_time >= fixed_time_step) {
				world->step(fixed_time_step, 4);
				accumulated_time -= fixed_time_step;
			}

			entity_registry
					.view<vis::physics::RigidBody, Ball>() //
					.each([&](const vis::physics::RigidBody& rb, auto) {
						auto pos = rb.get_transform().position;
						if (pos.y < -screen_proj.half_world_extent.x) {
							std::println("Player won");
							// Signal end of game
						}
						if (pos.y > screen_proj.half_world_extent.x) {
							std::println("AI won");
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

			const auto half_screen_extent = screen_proj.half_world_extent;
			constexpr auto half_pad_thickness = pad_thickness / 2.0f;
			constexpr auto x_offset = vis::vec2{half_pad_thickness, 0.0f} + 0.2f;
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
		}

		void add_player(vis::vec2 half_extent, vis::vec2 pos, vis::vec4 color) {
			constexpr auto origin = vis::vec2{0.0f, 0.0f};
			auto pad = entity_registry.create();
			entity_registry.emplace<Player>(pad);
			entity_registry.emplace<InputComponent>(pad, InputComponent{});
			entity_registry.emplace<vis::mesh::Mesh>(pad, vis::mesh::create_rectangle_shape(origin, half_extent, color));
			auto& transform = entity_registry.emplace<vis::physics::Transformation>(pad, vis::physics::Transformation{
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
			entity_registry.emplace<Ai>(pad);
			entity_registry.emplace<vis::mesh::Mesh>(pad, vis::mesh::create_rectangle_shape(origin, half_extent, color));

			vis::physics::RigidBodyDef body_def;
			body_def							 //
					.set_position(pos) //
					.set_body_type(vis::physics::BodyType::kinematic);
			auto& rigid_body = entity_registry.emplace<vis::physics::RigidBody>(pad, world->create_body(body_def));

			auto wall_box = vis::physics::create_box2d(half_extent);
			vis::physics::ShapeDef wall_shape;
			rigid_body.create_shape(wall_shape, wall_box);
		}

		void add_ball(float radius, vis::vec2 pos, vis::vec2 vel, vis::vec4 color) {
			auto ball = entity_registry.create();

			entity_registry.emplace<Ball>(ball);
			entity_registry.emplace<vis::mesh::Mesh>(ball, vis::mesh::create_regular_shape({}, radius, color, 10));

			auto circle = vis::physics::Circle{
					.center = {},
					.radius = radius,
			};

			vis::physics::RigidBodyDef body_def;
			body_def.set_position(pos)
					.set_body_type(vis::physics::BodyType::dynamic)
					.set_linear_velocity(vel)
					.set_is_bullet(true);

			auto& rigid_body = entity_registry.emplace<vis::physics::RigidBody>(ball, vis::physics::RigidBody{
																																										world->create_body(body_def),
																																								});
			vis::physics::ShapeDef shape_def;
			shape_def
					.set_restitution(1.0) //
					.set_friction(1.0f);
			rigid_body.create_shape(shape_def, circle);
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
			vis::physics::ShapeDef wall_shape;
			rigid_body.create_shape(wall_shape, wall_box);
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

		std::optional<vis::opengl::Program> program{};
		vis::ecs::registry entity_registry;
		vis::ScreenProjection screen_proj;

		std::optional<vis::physics::World> world;

		static constexpr auto pad_thickness = 0.6f;
		static constexpr auto pad_length = 3.0f;
		static constexpr auto ball_radius = 0.4f;
		static constexpr auto half_wall_thickness = 0.3f;
	};

	} // namespace Game
}
