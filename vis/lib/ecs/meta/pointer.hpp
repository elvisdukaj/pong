// IWYU pragma: always_keep

#ifndef ENTT_META_POINTER_HPP
#define ENTT_META_POINTER_HPP

#include "type_traits.hpp"
#if not defined(ENTT_IMPORT_STD)
#include <memory>
#include <type_traits>
#else
import std;
#endif

namespace vis::ecs {

/**
 * @brief Makes plain pointers pointer-like types for the meta system.
 * @tparam Type Element type.
 */
template <typename Type> struct is_meta_pointer_like<Type*> : std::true_type {};

/**
 * @brief Partial specialization used to reject pointers to arrays.
 * @tparam Type Type of elements of the array.
 * @tparam N Number of elements of the array.
 */
template <typename Type, std::size_t N>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
struct is_meta_pointer_like<Type (*)[N]> : std::false_type {};

/**
 * @brief Makes `std::shared_ptr`s of any type pointer-like types for the meta
 * system.
 * @tparam Type Element type.
 */
template <typename Type> struct is_meta_pointer_like<std::shared_ptr<Type>> : std::true_type {};

/**
 * @brief Makes `std::unique_ptr`s of any type pointer-like types for the meta
 * system.
 * @tparam Type Element type.
 * @tparam Args Other arguments.
 */
template <typename Type, typename... Args>
struct is_meta_pointer_like<std::unique_ptr<Type, Args...>> : std::true_type {};

/**
 * @brief Specialization for self-proclaimed meta pointer like types.
 * @tparam Type Element type.
 */
template <typename Type>
struct is_meta_pointer_like<Type, std::void_t<typename Type::is_meta_pointer_like>> : std::true_type {};

} // namespace vis::ecs

#endif
