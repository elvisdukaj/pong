#ifndef ENTT_CORE_RANGES_HPP
#define ENTT_CORE_RANGES_HPP

#if defined(ENTT_IMPORT_STD)
import std;
#endif

#if not defined(ENTT_IMPORT_STD) && __has_include(<version>)
#include <version>
#endif

#if defined(__cpp_lib_ranges)
#include "iterator.hpp"
#if not defined(ENTT_IMPORT_STD)
#include <ranges>
#endif

template <class... Args>
inline constexpr bool std::ranges::enable_borrowed_range<vis::ecs::iterable_adaptor<Args...>>{true};

template <class... Args> inline constexpr bool std::ranges::enable_view<vis::ecs::iterable_adaptor<Args...>>{true};

#endif
#endif