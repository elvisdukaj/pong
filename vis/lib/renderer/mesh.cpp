module;

#include <GL/glew.h>

#include <cassert>
#include <vector>

export module vis:mesh;

import std;
import :opengl;

#ifdef NDEBUG
#define CHECK_LAST_GL_CALL
#else
#define CHECK_LAST_GL_CALL                                                                                             \
	do {                                                                                                                 \
		auto err = glGetError();                                                                                           \
		if (err) {                                                                                                         \
			std::println("[{}][{}:{}] An OpenGL Error occurred", __FILE__, __func__, __LINE__);                              \
			assert(err == 0);                                                                                                \
		}                                                                                                                  \
	} while (false)
#endif

export namespace vis::mesh {

struct Vertex {
	vec2 pos;
	vec4 color;
};

struct VertexDescription {
	GLuint index;
	GLint size;
	GLenum type;
	GLboolean normalized;
	GLsizei stride;
	const void* pointer;
};

struct DrawDescription {
	GLenum mode;
	GLint first;
	GLsizei vertex_count;
};

class MeshShader {
public:
	MeshShader()
			: program{*gl::ProgramBuilder{}
										 .add_shader(gl::Shader::create(gl::ShaderType::vertex, R"(
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
										 .add_shader(gl::Shader::create(gl::ShaderType::fragment, R"(
#version 410 core

in vec4 vertex_color;
out vec4 fragment_color;

void main()
{
//    FragColor = vec4(0.0f, 0.5f, 0.2f, 1.0f);
		fragment_color = vertex_color;
}
      )"))
										 .build()} {}

	MeshShader(MeshShader&) = delete;
	MeshShader& operator=(MeshShader&) = delete;

	MeshShader(MeshShader&&) = default;
	MeshShader& operator=(MeshShader&&) = default;

	MeshShader& set_model_view_projection(const mat4& m) {
		program.set_uniform("model_view_projection", m);
		return *this;
	}

	void bind() const {
		program.use();
	}

	void unbind() const {
		program.unbind();
	}

private:
	gl::Program program;
};

class Mesh {
public:
	explicit Mesh(const std::vector<Vertex>& vertexes, const std::vector<VertexDescription>& vertex_descriptors,
								const DrawDescription& draw_descriptor)
			: vao{}, vbo{GL_ARRAY_BUFFER}, vertex_descriptors{vertex_descriptors}, draw_descriptor{draw_descriptor} {
		vao.bind();
		vbo.bind();

		vbo.data(begin(vertexes), end(vertexes), GL_STATIC_DRAW);

		for (auto& vertex_descriptor : vertex_descriptors) {
			glEnableVertexAttribArray(vertex_descriptor.index);
			CHECK_LAST_GL_CALL;

			glVertexAttribPointer(vertex_descriptor.index, vertex_descriptor.size, vertex_descriptor.type,
														vertex_descriptor.normalized, vertex_descriptor.stride, vertex_descriptor.pointer);
		}

		vbo.unbind();
		vao.unbind();
	}

	Mesh(Mesh&) = delete;
	Mesh& operator=(Mesh&) = delete;
	Mesh(Mesh&&) = default;
	Mesh& operator=(Mesh&&) = default;

	void bind() const {
		vao.bind();
	}

	void unbind() const {
		vao.unbind();
	}

	void draw([[maybe_unused]] const MeshShader& mesh_shader) const {
		bind();
		glDrawArrays(draw_descriptor.mode, draw_descriptor.first, draw_descriptor.vertex_count);
		unbind();
	}

private:
	gl::VertexArrayObject vao;
	gl::VertexBufferObject vbo;
	std::vector<VertexDescription> vertex_descriptors;
	DrawDescription draw_descriptor;
};

Mesh create_line_mesh(vec2 p1, vec2 p2, const vec4& color) {
	std::vector<Vertex> vertexes = {{p1, color}, {p2, color}};

	auto draw_description = DrawDescription{
			.mode = GL_LINES,
			.first = 0,
			.vertex_count = static_cast<GLsizei>(vertexes.size()),
	};
	auto vertex_descriptions = std::vector<VertexDescription>{
			VertexDescription{
					.index = 0,
					.size = 2,
					.type = GL_FLOAT,
					.normalized = GL_FALSE,
					.stride = sizeof(Vertex),
					.pointer = nullptr,
			},
			VertexDescription{
					.index = 1,
					.size = 4,
					.type = GL_FLOAT,
					.normalized = GL_FALSE,
					.stride = sizeof(Vertex),
					.pointer = reinterpret_cast<void*>(sizeof(vec2)),
			},
	};
	return Mesh{vertexes, vertex_descriptions, draw_description};
}

Mesh create_regular_shape(const vec2& center, float radius, const vec4& color, int num_vertices = 6) {
	const float theta_step = 2.0f * std::numbers::pi_v<float> / static_cast<float>(num_vertices);

	using VertexVector = std::vector<Vertex>;

	VertexVector vertexes;
	vertexes.reserve(static_cast<std::size_t>(num_vertices + 2)); // plus one for the center, and one for closing
	vertexes.emplace_back(center, color);
	for (int i = 0; i != num_vertices; i++) {
		auto angle = -theta_step * static_cast<float>(i);
		vertexes.emplace_back(vec2{std::cos(angle), std::sin(angle)} * radius + center, color);
	};
	vertexes.emplace_back(vec2{center.x + radius, center.y}, color);

	auto draw_description = DrawDescription{
			.mode = GL_TRIANGLE_FAN,
			.first = 0,
			.vertex_count = static_cast<GLsizei>(vertexes.size()),
	};
	auto vertex_descriptions = std::vector<VertexDescription>{
			VertexDescription{
					.index = 0,
					.size = 2,
					.type = GL_FLOAT,
					.normalized = GL_FALSE,
					.stride = sizeof(Vertex),
					.pointer = nullptr,
			},
			VertexDescription{
					.index = 1,
					.size = 4,
					.type = GL_FLOAT,
					.normalized = GL_FALSE,
					.stride = sizeof(Vertex),
					.pointer = reinterpret_cast<void*>(sizeof(vec2)),
			},
	};
	return Mesh{vertexes, vertex_descriptions, draw_description};
}

Mesh create_rectangle_shape(const vec2& center, const vec2& half_extent, vec4 color = vec4{}) {
	vec2 up = vec2(0.0f, half_extent.y);
	vec2 down = vec2(0.0f, -half_extent.y);
	vec2 left = vec2(-half_extent.x, 0.0f);
	vec2 right = vec2(half_extent.x, 0.0f);

	std::vector<Vertex> vertexes;
	vertexes.reserve(6);
	vertexes.emplace_back(center + down + left, color);
	vertexes.emplace_back(center + down + right, color);
	vertexes.emplace_back(center + up + right, color);
	vertexes.emplace_back(center + down + left, color);
	vertexes.emplace_back(center + up + right, color);
	vertexes.emplace_back(center + up + left, color);

	auto draw_description = DrawDescription{
			.mode = GL_TRIANGLES,
			.first = 0,
			.vertex_count = static_cast<GLsizei>(vertexes.size()),
	};
	auto vertex_descriptions = std::vector<VertexDescription>{
			VertexDescription{
					.index = 0,
					.size = 2,
					.type = GL_FLOAT,
					.normalized = GL_FALSE,
					.stride = sizeof(Vertex),
					.pointer = nullptr,
			},
			VertexDescription{
					.index = 1,
					.size = 4,
					.type = GL_FLOAT,
					.normalized = GL_FALSE,
					.stride = sizeof(Vertex),
					.pointer = reinterpret_cast<void*>(sizeof(vec2)),
			},
	};
	return Mesh{vertexes, vertex_descriptions, draw_description};
}

} // namespace vis::mesh