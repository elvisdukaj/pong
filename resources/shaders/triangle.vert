#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 position;

vec2 vertexes[3] = {
vec2(0.0, 0.8),
vec2(-0.8, -0.8),
vec2(0.8, -0.8)
};

void main() {
    gl_Position = vec4(vertexes[gl_VertexIndex], 0.0, 1.0);
}
