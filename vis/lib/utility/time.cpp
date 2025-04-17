module;

#include <SDL3/SDL.h>
#include <chrono>

export module vis.chrono;

import std;
import vis.math;

export namespace vis {

namespace chrono {

using microseconds = std::chrono::duration<float, std::micro>;
using milliseconds = std::chrono::duration<float, std::milli>;
using seconds = std::chrono::duration<float>;

struct Clock {
	using duration = milliseconds;
	using rep = duration::rep;
	using period = duration::period;
	using time_point = std::chrono::time_point<Clock, duration>;
	static constexpr bool is_steady = true;

	static Clock::time_point now() noexcept {
		return Clock::time_point{Clock::duration{static_cast<float>(SDL_GetTicks())}};
	}
};

class Timer {
public:
	using clock = vis::chrono::Clock;

	Timer() : start_time_point(clock::now()) {}

	void reset() {
		start_time_point = clock::now();
	}

	clock::duration elapsed() const {
		return Clock::now() - start_time_point;
	}

private:
	clock::time_point start_time_point;
};
} // namespace chrono

inline namespace literals { inline namespace chrono_literals {

constexpr chrono::milliseconds operator""_ms(long double milli_sec) {
	return chrono::milliseconds(static_cast<chrono::milliseconds::rep>(milli_sec));
}

constexpr chrono::seconds operator""_s(long double sec) {
	return chrono::seconds(static_cast<chrono::seconds::rep>(sec));
}

}} // namespace literals::chrono_literals
} // namespace vis

export {
	vis::vec2 operator*(vis::vec2 v, vis::chrono::seconds d) {
		return v * d.count();
	}

	constexpr vis::chrono::seconds operator/(vis::chrono::seconds lhs, vis::chrono::seconds rhs) {
		return vis::chrono::seconds{lhs.count() / rhs.count()};
	}

	constexpr vis::chrono::milliseconds operator/(vis::chrono::milliseconds lhs, vis::chrono::milliseconds rhs) {
		return vis::chrono::milliseconds{lhs.count() / rhs.count()};
	}

	constexpr vis::chrono::seconds operator*(vis::chrono::seconds lhs, vis::chrono::seconds rhs) {
		return vis::chrono::seconds{lhs.count() / rhs.count()};
	}

	constexpr vis::chrono::milliseconds operator*(vis::chrono::milliseconds lhs, vis::chrono::milliseconds rhs) {
		return vis::chrono::milliseconds{lhs.count() / rhs.count()};
	}
}

export namespace std {
template <> struct formatter<vis::chrono::milliseconds> {
	constexpr auto parse(std::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(vis::chrono::milliseconds t, std::format_context& ctx) const {
		return std::format_to(ctx.out(), "{}ms", t.count());
	}
};

template <> struct formatter<vis::chrono::seconds> {
	constexpr auto parse(std::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(vis::chrono::seconds t, std::format_context& ctx) const {
		return std::format_to(ctx.out(), "{}s", t.count());
	}
};
} // namespace std