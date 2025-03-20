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

inline vis::vec2 get_random_direction(float min_angle, float max_angle) {
	float a = get_random(min_angle, max_angle);
	return vis::vec2(std::cos(a), std::sin(a));
}
} // namespace vis