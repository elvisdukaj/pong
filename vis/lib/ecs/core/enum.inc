

export namespace vis {
namespace ecs {
using ::entt::enum_as_bitmask;
using ::entt::enum_as_bitmask_v;
}}

// global operators
export {
	// TODO: is this working? I want to export all global operator from entt
	using ::operator|;
	using ::operator&;
	using ::operator^;
	using ::operator~;
	using ::operator!;
	using ::operator|=;
	using ::operator&=;
	using ::operator^=;
}
