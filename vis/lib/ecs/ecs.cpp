module;

#include "ecs/entt.hpp"

export module vis.ecs;

export import std;

#include "ecs/container/dense_map.inc"
#include "ecs/container/dense_set.inc"
#include "ecs/container/fwd.inc"
#include "ecs/container/table.inc"
#include "ecs/core/algorithm.inc"
#include "ecs/core/any.inc"
#include "ecs/core/attribute.inc"
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
#include "ecs/entity/component.inc"
#include "ecs/entity/entity.inc"
#include "ecs/entity/fwd.inc"
#include "ecs/entity/group.inc"
#include "ecs/entity/handle.inc"
#include "ecs/entity/helper.inc"
#include "ecs/entity/mixin.inc"
#include "ecs/entity/organizer.inc"
#include "ecs/entity/ranges.inc"
#include "ecs/entity/registry.inc"
#include "ecs/entity/runtime_view.inc"
#include "ecs/entity/snapshot.inc"
#include "ecs/entity/sparse_set.inc"
#include "ecs/entity/storage.inc"
#include "ecs/entity/view.inc"
#include "ecs/graph/adjacency_matrix.inc"
#include "ecs/graph/dot.inc"
#include "ecs/graph/flow.inc"
#include "ecs/graph/fwd.inc"
#include "ecs/locator/locator.inc"
#include "ecs/meta/adl_pointer.inc"
#include "ecs/meta/container.inc"
#include "ecs/meta/context.inc"
#include "ecs/meta/factory.inc"
#include "ecs/meta/fwd.inc"
#include "ecs/meta/meta.inc"
#include "ecs/meta/node.inc"
#include "ecs/meta/pointer.inc"
#include "ecs/meta/policy.inc"
#include "ecs/meta/range.inc"
#include "ecs/meta/resolve.inc"
#include "ecs/meta/template.inc"
#include "ecs/meta/type_traits.inc"
#include "ecs/meta/utility.inc"
#include "ecs/operators.inc"
#include "ecs/poly/fwd.inc"
#include "ecs/poly/poly.inc"
#include "ecs/process/fwd.inc"
#include "ecs/process/process.inc"
#include "ecs/process/scheduler.inc"
#include "ecs/resource/cache.inc"
#include "ecs/resource/fwd.inc"
#include "ecs/resource/loader.inc"
#include "ecs/resource/resource.inc"
#include "ecs/signal/delegate.inc"
#include "ecs/signal/dispatcher.inc"
#include "ecs/signal/emitter.inc"
#include "ecs/signal/fwd.inc"

#include "ecs/signal/sigh.inc"
#include "ecs/std.inc"