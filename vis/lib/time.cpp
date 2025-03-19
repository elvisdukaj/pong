module;

#include <SDL3/SDL.h>

export module vis:chrono;

import std;

export namespace vis::chrono {

struct Clock {
	typedef std::chrono::duration<float, std::milli> duration;
	typedef duration::rep rep;
	typedef duration::period period;
	typedef std::chrono::time_point<Clock, duration> time_point;
	static constexpr bool is_steady = true;

	static Clock::time_point now() noexcept {
		return Clock::time_point{Clock::duration{static_cast<float>(SDL_GetTicks()) / 1000.0f}};
	}
};

class Timer {
public:
	Timer() : start_time_point(Clock::now()) {}

	void reset() {
		start_time_point = Clock::now();
	}

	Clock::duration elapsed() const {
		return Clock::now() - start_time_point;
	}

private:
	Clock::time_point start_time_point;
};

} // namespace vis::chrono