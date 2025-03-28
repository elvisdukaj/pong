module;

#include <entt/entt.hpp>

export module vis:ecs;
export import std;

#include "ecs/container/dense_map.inc"
#include "ecs/container/dense_set.inc"
#include "ecs/container/fwd.inc"
#include "ecs/container/table.inc"
#include "ecs/core/algorithm.inc"
#include "ecs/core/any.inc"
#include "ecs/core/bit.inc"
#include "ecs/core/compressed_pair.inc"
#include "ecs/core/enum.inc"
#include "ecs/core/family.inc"
#include "ecs/core/fwd.inc"
#include "ecs/core/hashed_string.inc"
#include "ecs/core/ident.inc"
#include "ecs/core/iterator.inc"
#include "ecs/core/memory.inc"
#include "ecs/core/monostate.inc"
#include "ecs/core/ranges.inc"
#include "ecs/core/tuple.inc"
#include "ecs/core/type_info.inc"
#include "ecs/core/type_traits.inc"
#include "ecs/core/utility.inc"

#include "ecs/entity/entity.inc"
