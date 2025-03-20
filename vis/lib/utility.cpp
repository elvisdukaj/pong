module;

export module vis:utility;

import std;

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

} // namespace vis