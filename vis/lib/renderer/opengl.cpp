module;

#include <GL/glew.h>
#include <SDL3/SDL.h>

#include <cassert>

#ifdef NDEBUG
#define CHECK_LAST_GL_CALL
#else
#define CHECK_LAST_GL_CALL                                                                                             \
	do {                                                                                                                 \
		auto err = glGetError();                                                                                           \
		if (err) {                                                                                                         \
			std::println("[{}][{}:{}] An OpenGL Error occured", __FILE__, __func__, __LINE__);                               \
			assert(err);                                                                                                     \
		}                                                                                                                  \
	} while (false)
#endif

export module vis:opengl;

import std;
import :math;
import :window;

export namespace vis::gl {

struct VertexArrayObject {
	VertexArrayObject() {
		glGenVertexArrays(1, &id);
	}

	~VertexArrayObject() {
		if (id != 0)
			glDeleteVertexArrays(1, &id);
	}

	friend void swap(VertexArrayObject& lhs, VertexArrayObject& rhs) {
		std::swap(lhs.id, rhs.id);
	}

	VertexArrayObject(VertexArrayObject&) = delete;
	VertexArrayObject& operator=(VertexArrayObject&) = delete;

	VertexArrayObject(VertexArrayObject&& other) noexcept : id{0} {
		swap(*this, other);
	}

	VertexArrayObject& operator=(VertexArrayObject&& other) noexcept {
		swap(*this, other);
		return *this;
	}

	void bind() const {
		glBindVertexArray(id);
	}

	static void unbind() {
		glBindVertexArray(0);
	}

	explicit operator GLuint() const {
		return id;
	}

	GLuint id{};
};

struct VertexBufferObject {
	explicit VertexBufferObject(GLenum type) : type{type} {
		glGenBuffers(1, &id);
	}

	~VertexBufferObject() {
		if (id != 0)
			glDeleteBuffers(1, &id);
	}

	friend void swap(VertexBufferObject& lhs, VertexBufferObject& rhs) {
		std::swap(lhs.id, rhs.id);
	}

	VertexBufferObject(VertexBufferObject&) = delete;
	VertexBufferObject& operator=(VertexBufferObject&) = delete;

	VertexBufferObject(VertexBufferObject&& rhs) noexcept : id{0} {
		swap(*this, rhs);
	}

	VertexBufferObject& operator=(VertexBufferObject&& rhs) noexcept {
		swap(*this, rhs);
		return *this;
	}

	void bind() const {
		glBindBuffer(type, id);
		CHECK_LAST_GL_CALL;
	}

	void unbind() const {
		glBindBuffer(type, 0);
		CHECK_LAST_GL_CALL;
	}

	template <typename ConstRandomIterator> void data(ConstRandomIterator begin, ConstRandomIterator end, GLenum usage) {
		using value_type = typename std::iterator_traits<ConstRandomIterator>::value_type;

		constexpr auto value_type_size = sizeof(value_type);
		const auto element_count = static_cast<std::size_t>(std::distance(begin, end));
		const auto total_size_in_bytes = element_count * value_type_size;

		data(total_size_in_bytes, &(*begin), usage);
	}

	void data(std::size_t size, const void* data, GLenum usage) const {
		glBufferData(type, static_cast<GLsizei>(size), data, usage);
		CHECK_LAST_GL_CALL;
	}

	explicit operator GLuint() const {
		return id;
	}

	GLenum type{GL_VERTEX_ARRAY};
	GLuint id{};
};

enum class ShaderType {
	vertex,
	geometry,
	tesselation_evaluation,
	tesselation_control,
	compute,
	fragment,
};

GLenum to_opengl(ShaderType type) {
	switch (type) {
	case ShaderType::vertex:
		return GL_VERTEX_SHADER;
	case ShaderType::geometry:
		return GL_GEOMETRY_SHADER;
	case ShaderType::tesselation_evaluation:
		return GL_TESS_EVALUATION_SHADER;
	case ShaderType::tesselation_control:
		return GL_TESS_CONTROL_SHADER;
	case ShaderType::compute:
		return GL_COMPUTE_SHADER;
	case ShaderType::fragment:
		return GL_FRAGMENT_SHADER;

	default:
		std::unreachable();
	}
}

class Shader {
public:
	static Shader create(ShaderType type, std::string_view source) {
		return Shader{to_opengl(type), source};
	}

	friend void swap(Shader& lhs, Shader& rhs) {
		std::swap(lhs.type, rhs.type);
		std::swap(lhs.id, rhs.id);
	}

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	Shader(Shader&& other) noexcept : type{GL_INVALID_ENUM}, id{} {
		swap(*this, other);
	}

	Shader& operator=(Shader&& other) noexcept {
		swap(*this, other);
		return *this;
	}

	~Shader() {
		if (type != GL_INVALID_ENUM)
			glDeleteShader(id);
		CHECK_LAST_GL_CALL;
	}

	explicit operator GLuint() const {
		return id;
	}

private:
	explicit Shader(GLenum type, std::string_view source) : type{type}, id{glCreateShader(type)} {
		char const* source_pointer = source.data();
		glShaderSource(id, 1, &(source_pointer), nullptr);
		CHECK_LAST_GL_CALL;

		glCompileShader(id);
		CHECK_LAST_GL_CALL;

		// Check Vertex Shader
		GLint res = GL_FALSE;
		GLint info_log_len = 0;
		glGetShaderiv(id, GL_COMPILE_STATUS, &res);
		CHECK_LAST_GL_CALL;

		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_log_len);
		CHECK_LAST_GL_CALL;

		if (info_log_len > 0) {
			std::string message;
			message.resize(static_cast<std::size_t>(info_log_len) + 1, '\0');
			glGetShaderInfoLog(id, info_log_len, nullptr, message.data());
			CHECK_LAST_GL_CALL;

			std::println("Shader compilation error: {}", message);
		};
	}

private:
	GLenum type;
	GLuint id;
};

class Program {
public:
	static std::optional<Program> create(std::vector<Shader> shaders) {
		return Program{std::move(shaders)};
	}

	friend void swap(Program& lhs, Program& rhs) {
		std::swap(lhs.id, rhs.id);
	}

	Program(const Program&) = delete;
	Program& operator=(const Program&) = delete;

	Program(Program&& other) noexcept : id{0} {
		swap(*this, other);
	}

	Program& operator=(Program&& rhs) noexcept {
		std::swap(id, rhs.id);
		return *this;
	}

	~Program() {
		if (id != 0) {
			glDeleteShader(id);
			CHECK_LAST_GL_CALL;
		}
	}

	explicit operator GLuint() const {
		return id;
	}

	void use() const {
		glUseProgram(id);
	}

	void set_uniform(std::string_view name, const mat3& m) {
		// TODO: make it a concept for VectorConcept and MatrixConcept so we can write this as:
		// template<typename T> requires IsVector<T> or IsMatrix<T>
		const auto loc = get_or_update_uniform(name);
		glUniformMatrix3fv(loc, 1, GL_FALSE, gtc::value_ptr(m));
		CHECK_LAST_GL_CALL;
	}

	void set_uniform(std::string_view name, const mat4& m) {
		// TODO: make it a concept for VectorConcept and MatrixConcept so we can write this as:
		// template<typename T> requires IsVector<T> or IsMatrix<T>
		const auto loc = get_or_update_uniform(name);
		glUniformMatrix4fv(loc, 1, GL_FALSE, gtc::value_ptr(m));
		CHECK_LAST_GL_CALL;
	}

	static void unbind() {
		glUseProgram(0);
		CHECK_LAST_GL_CALL;
	}

private:
	explicit Program(std::vector<Shader>&& shaders) : id{glCreateProgram()} {
		for (const auto& shader : shaders) {
			glAttachShader(id, static_cast<GLuint>(shader));
			CHECK_LAST_GL_CALL;
		}

		glLinkProgram(id);
		CHECK_LAST_GL_CALL;

		GLint result = GL_FALSE;
		GLint info_log_len = 0;

		glGetProgramiv(id, GL_LINK_STATUS, &result);
		CHECK_LAST_GL_CALL;

		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &info_log_len);
		CHECK_LAST_GL_CALL;

		if (info_log_len > 0) {
			std::string message;
			message.resize(static_cast<std::size_t>(info_log_len + 1), '\0');

			glGetProgramInfoLog(id, info_log_len, nullptr, message.data());
			CHECK_LAST_GL_CALL;

			std::println("Link error: {}", message);
		}
	}

	[[nodiscard]] GLint get_uniform_id(std::string_view name) const {
		auto res = glGetUniformLocation(id, name.data());
		CHECK_LAST_GL_CALL;

		return res;
	}

	[[nodiscard]] GLint get_or_update_uniform(std::string_view name) {
		auto it = uniforms.find(name);
		if (it != uniforms.end()) {
			return it->second;
		}

		return uniforms[name] = get_uniform_id(name);
	}

private:
	GLuint id;
	std::map<std::string_view, GLint> uniforms;
};

class ProgramBuilder {
public:
	ProgramBuilder& add_shader(Shader&& shader) {
		shaders.emplace_back(std::move(shader));
		return *this;
	}

	[[nodiscard]] std::optional<Program> build() {
		return Program::create(std::move(shaders));
	}

private:
	std::vector<Shader> shaders;
};

struct DrawDescription {
	GLenum mode;
	GLint first;
	GLsizei vertex_count;
};

class OpenGLRenderer {
public:
	static std::expected<OpenGLRenderer, std::string> create(vis::Window* window) {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

#ifndef NDEBUG
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

		SDL_GLContext opengl_context = SDL_GL_CreateContext(static_cast<SDL_Window*>(*window));
		if (not opengl_context) {
			return std::unexpected(std::format("Unable to initialize OpenGL: {}", SDL_GetError()));
			// return nullptr;
		}

		if (not SDL_GL_MakeCurrent(static_cast<SDL_Window*>(*window), opengl_context)) {
			SDL_GL_DestroyContext(opengl_context);
			return std::unexpected("It's not possible to init the graphic");
		}

		auto glewStatus = glewInit();
		if (glewStatus != GLEW_OK) {
			return std::unexpected("Unable to initialize OpenGL");
		}

		if (not SDL_GL_SetSwapInterval(1)) {
			SDL_GL_DestroyContext(opengl_context);
			std::unexpected("It's not possible to set the vsync");
		}

		return OpenGLRenderer{window, opengl_context};
	}

	~OpenGLRenderer() {
		if (context) {
			SDL_GL_DestroyContext(context);
			context = nullptr;
		}
	}

	friend void swap(OpenGLRenderer& lhs, OpenGLRenderer& rhs) {
		std::swap(lhs.window, rhs.window);
		std::swap(lhs.context, rhs.context);
	}

	OpenGLRenderer(OpenGLRenderer&) = delete;
	OpenGLRenderer& operator=(OpenGLRenderer&) = delete;

	OpenGLRenderer(OpenGLRenderer&& other) noexcept : window{nullptr}, context{nullptr} {
		swap(*this, other);
	}

	OpenGLRenderer& operator=(OpenGLRenderer&& other) noexcept {
		swap(*this, other);
		return *this;
	}

	void set_clear_color(const vec4& color) {
		glClearColor(color.r, color.g, color.b, color.a);
		CHECK_LAST_GL_CALL;
	}

	void clear() {
		glClear(GL_COLOR_BUFFER_BIT);
		CHECK_LAST_GL_CALL;
	}

	void render() const {
		SDL_GL_SwapWindow(static_cast<SDL_Window*>(*window));
	}

	void set_viewport(int x, int y, int width, int height) {
		glViewport(x, y, width, height);
		CHECK_LAST_GL_CALL;
	}

	std::string show_info() {
		auto renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
		auto version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

		return std::format("viz engine version 0.1\n"
											 "Renderer {}\n"
											 "OpenGL version supported {}",
											 renderer, version);
	}

private:
	OpenGLRenderer(vis::Window* window, SDL_GLContext context) : window{window}, context(context) {}

private:
	vis::Window* window;
	SDL_GLContext context;
};

} // namespace vis::opengl