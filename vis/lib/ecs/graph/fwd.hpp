#ifndef ENTT_GRAPH_FWD_HPP
#define ENTT_GRAPH_FWD_HPP

#include "../core/fwd.hpp"
#if not defined(ENTT_IMPORT_STD)
#include <cstddef>
#include <memory>
#else
import std;
#endif

namespace vis::ecs {

/*! @brief Undirected graph category tag. */
struct directed_tag {};

/*! @brief Directed graph category tag. */
struct undirected_tag : directed_tag {};

template <typename, typename = std::allocator<std::size_t>> class adjacency_matrix;

template <typename = std::allocator<id_type>> class basic_flow;

/*! @brief Alias declaration for the most common use case. */
using flow = basic_flow<>;

} // namespace vis::ecs

#endif
