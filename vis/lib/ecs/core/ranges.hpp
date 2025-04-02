#ifndef ENTT_CORE_RANGES_HPP
#define ENTT_CORE_RANGES_HPP

#if __has_include(<version>)
#include <version>
#
#if defined(__cpp_lib_ranges)
#include "iterator.hpp"
#include <ranges>

template <class... Args>
inline constexpr bool std::ranges::enable_borrowed_range<vis::ecs::iterable_adaptor<Args...>>{true};

template <class... Args> inline constexpr bool std::ranges::enable_view<vis::ecs::iterable_adaptor<Args...>>{true};

#endif
#endif

#endif