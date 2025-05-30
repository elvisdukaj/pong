#ifndef ENTT_POLY_FWD_HPP
#define ENTT_POLY_FWD_HPP

#if not defined(ENTT_IMPORT_STD)
#include <cstddef>
#else
import std;
#endif

namespace vis::ecs {

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
template <typename, std::size_t Len = sizeof(double[2]), std::size_t = alignof(double[2])> class basic_poly;

/**
 * @brief Alias declaration for the most common use case.
 * @tparam Concept Concept descriptor.
 */
template <typename Concept> using poly = basic_poly<Concept>;

} // namespace vis::ecs

#endif
