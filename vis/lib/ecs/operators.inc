export namespace vis::ecs {
using vis::ecs::operator==;
using vis::ecs::operator!=;
using vis::ecs::operator<;
using vis::ecs::operator<=;
using vis::ecs::operator>;
using vis::ecs::operator>=;

using vis::ecs::operator+;
} // namespace vis::ecs

// global operators
export {
	// from entt/core/enum.h
	using ::operator|;
	using ::operator&;
	using ::operator^;
	using ::operator~;
	using ::operator!;
	using ::operator|=;
	using ::operator&=;
	using ::operator^=;
}
