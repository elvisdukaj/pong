module;

#include <boost/sml.hpp>

export module game:ai;

import :events;
import :constants;

import std;
import vis;

export namespace Game {

struct AiContext {
	vis::vec2 ai_direction;
};

auto defend = [](const DefendEvent& event, AiContext& context) {
	const auto& [pad_pos] = event;
	if (std::abs(pad_pos.y) < 0.3) {
		context.ai_direction = vis::vec2{0.0f, 0.0f};
		return;
	}

	context.ai_direction = (pad_pos.y < 0.0f) ? up : down;
};

auto follow = [](const FollowingEvent& event, AiContext& context) {
	const auto& [pad_pos, ball_y_pos] = event;
	context.ai_direction = (ball_y_pos > pad_pos.y) ? up : down;
};

struct AiState {
	auto operator()() const {
		using namespace boost::sml;

		return make_transition_table(
				// clang-format off
				* "defend"_s	+ event<FollowingEvent>		/ follow						= "follow"_s,
					"defend"_s	+ event<DefendEvent>			/ defend						= "defend"_s,
					"follow"_s	+ event<FollowingEvent>		/ follow			    	= "follow"_s,
					"follow"_s	+ event<DefendEvent>			/ defend						= "defend"_s
				// clang-format on
		);
	}
};

template <typename T> using sm = boost::sml::sm<T>;

} // namespace Game

export {
	using ::boost::sml::operator""_s;
}
