cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(glew CONFIG REQUIRED)
find_package(VulkanLoader CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

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
        vulkan.cpp

        PRIVATE FILE_SET HEADERS
        FILES

        ecs/container/dense_map.inc
        ecs/container/dense_set.inc
        ecs/container/table.inc
        ecs/container/fwd.inc
        ecs/core/algorithm.inc
        ecs/core/any.inc
        ecs/core/attribute.inc
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
        ecs/entity/handle.inc
        ecs/entity/mixin.inc
        ecs/entity/helper.inc
        ecs/entity/organizer.inc
        ecs/entity/ranges.inc
        ecs/entity/registry.inc
        ecs/entity/runtime_view.inc
        ecs/entity/snapshot.inc
        ecs/entity/sparse_set.inc
        ecs/entity/storage.inc
        ecs/entity/view.inc
        ecs/graph/adjacency_matrix.inc
        ecs/graph/dot.inc
        ecs/graph/flow.inc
        ecs/graph/fwd.inc
        ecs/locator/locator.inc
        ecs/meta/adl_pointer.inc
        ecs/meta/container.inc
        ecs/meta/context.inc
        ecs/meta/factory.inc
        ecs/meta/fwd.inc
        ecs/meta/meta.inc
        ecs/meta/node.inc
        ecs/meta/pointer.inc
        ecs/meta/policy.inc
        ecs/meta/range.inc
        ecs/meta/resolve.inc
        ecs/meta/template.inc
        ecs/meta/type_traits.inc
        ecs/meta/utility.inc
        ecs/poly/fwd.inc
        ecs/poly/poly.inc
        ecs/process/fwd.inc
        ecs/process/process.inc
        ecs/process/scheduler.inc
        ecs/resource/cache.inc
        ecs/resource/fwd.inc
        ecs/resource/loader.inc
        ecs/resource/resource.inc
        ecs/signal/delegate.inc
        ecs/signal/dispatcher.inc
        ecs/signal/emitter.inc
        ecs/signal/fwd.inc
        ecs/signal/sigh.inc
)

target_compile_definitions(vis_obj PUBLIC
        "SDL_MAIN_USE_CALLBACKS=1"
        ENTT_STANDARD_CPP
        VULKAN_HPP_RAII_NO_EXCEPTIONS
        $<$<STREQUAL:$<PLATFORM_ID>,Linux>:VK_USE_PLATFORM_WAYLAND_KHR>
        $<$<STREQUAL:$<PLATFORM_ID>,Windows>:VK_USE_PLATFORM_WIN32_KHR>
        $<$<STREQUAL:$<PLATFORM_ID>,Darwin>:VK_USE_PLATFORM_METAL_EXT>
)

target_link_libraries(vis_obj PUBLIC SDL3::SDL3 glm::glm GLEW::GLEW box2d::box2d Vulkan::Loader yaml-cpp::yaml-cpp)

target_add_extra_warnings(vis_obj PUBLIC)
target_set_warnings_as_error(vis_obj)