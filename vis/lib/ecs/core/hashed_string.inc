export namespace vis {
namespace ecs {

using ::entt::basic_hashed_string;
using ::entt::operator==;
using ::entt::operator!=;
using ::entt::operator<;
using ::entt::operator<=;
using ::entt::operator>;
using ::entt::operator>=;

inline namespace literals {

using ::entt::literals::operator ""_hs;
using ::entt::literals::operator ""_hws;

} // namespace literals

}}