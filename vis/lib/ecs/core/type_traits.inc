export namespace vis {
namespace ecs {

using ::entt::choice;
using ::entt::choice_t;
using ::entt::type_identity;
using ::entt::type_identity_t;
using ::entt::size_of;
using ::entt::size_of_v;
using ::entt::unpack_as_type;
using ::entt::unpack_as_value;
using ::entt::integral_constant;
using ::entt::tag;
using ::entt::type_list;
using ::entt::type_list_element;
using ::entt::type_list_element_t;
using ::entt::type_list_index;
using ::entt::type_list_index_v;
using ::entt::type_list_cat;
using ::entt::type_list_cat_t;
using ::entt::type_list_unique;
using ::entt::type_list_unique_t;
using ::entt::type_list_contains;
using ::entt::type_list_contains_v;
using ::entt::type_list_diff;
using ::entt::type_list_diff_t;
using ::entt::type_list_transform;
using ::entt::type_list_transform_t;
using ::entt::value_list;
using ::entt::value_list_element;
using ::entt::value_list_element_t;
using ::entt::value_list_index;
using ::entt::value_list_index_v;
using ::entt::value_list_cat;
using ::entt::value_list_cat_t;
using ::entt::value_list_unique;
using ::entt::value_list_unique_t;
using ::entt::value_list_contains;
using ::entt::value_list_contains_v;
using ::entt::value_list_diff;
using ::entt::value_list_diff_t;
using ::entt::is_applicable;
using ::entt::is_applicable_v;
using ::entt::is_applicable_r;
using ::entt::is_applicable_r_v;
using ::entt::is_complete;
using ::entt::is_complete_v;
using ::entt::is_iterator;
using ::entt::is_iterator_v;
using ::entt::is_ebco_eligible;
using ::entt::is_ebco_eligible_v;
using ::entt::is_transparent;
using ::entt::is_transparent_v;
using ::entt::is_equality_comparable;
using ::entt::is_equality_comparable_v;
using ::entt::constness_as;
using ::entt::constness_as_t;
using ::entt::member_class;
using ::entt::member_class_t;
using ::entt::nth_argument;
using ::entt::nth_argument_t;

using ::entt::operator+;

}}

export namespace std{
	using ::std::tuple_size;
	using ::std::tuple_element;
}