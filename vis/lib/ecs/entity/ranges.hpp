#ifndef ENTT_ENTITY_RANGES_HPP
#define ENTT_ENTITY_RANGES_HPP

#if defined(ENTT_IMPORT_STD)
import std;
#endif

#if not defined(ENTT_IMPORT_STD) and __has_include(<version>)
#include <version>
#endif

#if defined(__cpp_lib_ranges)
#include "fwd.hpp"
#if not defined(ENTT_IMPORT_STD)
#include <ranges>
#endif

template <class... Args> inline constexpr bool std::ranges::enable_borrowed_range<vis::ecs::basic_view<Args...>>{true};

template <class... Args> inline constexpr bool std::ranges::enable_borrowed_range<vis::ecs::basic_group<Args...>>{true};

template <class... Args> inline constexpr bool std::ranges::enable_view<vis::ecs::basic_view<Args...>>{true};

template <class... Args> inline constexpr bool std::ranges::enable_view<vis::ecs::basic_group<Args...>>{true};

#endif
#endif