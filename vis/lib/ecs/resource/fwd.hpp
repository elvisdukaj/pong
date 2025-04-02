#ifndef ENTT_RESOURCE_FWD_HPP
#define ENTT_RESOURCE_FWD_HPP

#include <memory>

namespace vis::ecs {

template <typename> struct resource_loader;

template <typename Type, typename = resource_loader<Type>, typename = std::allocator<Type>> class resource_cache;

template <typename> class resource;

} // namespace vis::ecs

#endif
