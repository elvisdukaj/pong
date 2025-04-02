#ifndef ENTT_ENTITY_RANGES_HPP
#define ENTT_ENTITY_RANGES_HPP

#if __has_include(<version>)
#include <version>
#
#if defined(__cpp_lib_ranges)
#include "fwd.hpp"
#include <ranges>

template <class... Args> inline constexpr bool std::ranges::enable_borrowed_range<vis::ecs::basic_view<Args...>>{true};

template <class... Args> inline constexpr bool std::ranges::enable_borrowed_range<vis::ecs::basic_group<Args...>>{true};

template <class... Args> inline constexpr bool std::ranges::enable_view<vis::ecs::basic_view<Args...>>{true};

template <class... Args> inline constexpr bool std::ranges::enable_view<vis::ecs::basic_group<Args...>>{true};

#endif
#endif

#endif