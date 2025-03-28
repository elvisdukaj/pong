cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

add_library(vis_obj OBJECT)
add_library(vis_obj::vis_obj ALIAS vis_obj)

target_sources(vis_obj
        PUBLIC FILE_SET CXX_MODULES FILES
        app.cpp
        opengl.cpp
        vis.cpp
        math.cpp
        entt.cpp
        mesh.cpp
        physic.cpp
        time.cpp
        utility.cpp
        window.cpp

        PRIVATE FILE_SET HEADERS
        FILES
        # ecs
        ecs/container/dense_map.inc
        ecs/container/dense_set.inc
        ecs/container/fwd.inc
        ecs/container/table.inc
        ecs/core/algorithm.inc
        ecs/core/any.inc
        ecs/core/bit.inc
        ecs/core/compressed_pair.inc
        ecs/core/enum.inc
        ecs/core/family.inc
        ecs/core/fwd.inc
        ecs/core/hashed_string.inc
        ecs/core/ident.inc
        ecs/core/iterator.inc
        ecs/core/memory.inc
        ecs/core/monostate.inc
        ecs/core/ranges.inc
        ecs/core/tuple.inc
        ecs/core/type_info.inc
        ecs/core/type_traits.inc
        ecs/core/utility.inc
        ecs/entity/component.inc
        ecs/entity/entity.inc
        ecs/entity/fwd.inc
        ecs/entity/group.inc
        ecs/entity/helper.inc
        ecs/entity/mixin.inc
        ecs/entity/organizer.inc
        ecs/entity/ranges.inc
        ecs/entity/registry.inc
        ecs/entity/runtime_view.inc
        ecs/entity/snapshot.inc
        ecs/entity/sparse_set.inc
        ecs/entity/storage.inc

        ecs/signal/delegate.inc
        ecs/signal/dispatcher.inc
        ecs/signal/emitter.inc
        ecs/signal/fwd.inc
        ecs/signal/sigh.inc
)

target_compile_definitions(vis_obj PUBLIC "SDL_MAIN_USE_CALLBACKS=1" ENTT_STANDARD_CPP)
target_link_libraries(vis_obj PUBLIC SDL3::SDL3 EnTT::EnTT glm::glm GLEW::GLEW box2d::box2d)

target_add_extra_warnings(vis_obj PUBLIC)
target_set_warnings_as_error(vis_obj)