module;

export module vis:utility;

import std;

import :math;

export namespace vis {
// Function to generate a random float between min and max
inline float get_random(float min, float max) {
	static std::mt19937 rng(std::random_device{}());			// Random number generator
	std::uniform_real_distribution<float> dist(min, max); // Uniform distribution
	return dist(rng);																			// Generate the random number
}

inline float get_random(float val) {
	return get_random(-val, +val);
}

inline vec2 get_random_direction(vec2 vec, float min_angle, float max_angle) {
	float a = get_random(min_angle, max_angle);

	mat4 rot = gtc::identity<mat4>();
	rot = gtc::rotate(rot, a, vec3{vec.x, vec.y, 1.0f});
	auto rotated = vec4{vec.x, vec.y, 0.0f, 1.0f} * rot;

	return normalize(vec2{rotated.x, rotated.y});
}
} // namespace vis