#ifndef ENTT_PROCESS_FWD_HPP
#define ENTT_PROCESS_FWD_HPP

#if not defined(ENTT_IMPORT_STD)
#include <cstdint>
#include <memory>
#else
import std;
#endif

namespace vis::ecs {

template <typename, typename> class process;

template <typename = std::uint32_t, typename = std::allocator<void>> class basic_scheduler;

/*! @brief Alias declaration for the most common use case. */
using scheduler = basic_scheduler<>;

} // namespace vis::ecs

#endif
