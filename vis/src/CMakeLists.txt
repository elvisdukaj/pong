cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

add_library(vis)
add_library(vis::vis ALIAS vis)

target_link_libraries(vis PUBLIC vis_obj::vis_obj)
target_set_warnings_as_error(vis)

add_spirv_modules(triangle_spirv
        SOURCE_DIR ${RESOURCE_SHADER_DIR}
        BINARY_DIR ${CMAKE_BINARY_DIR}/resources/shader
        SOURCES triangle.vert
        OPTIONS
        $<$<CONFIG:Debug>:-Od -g>
        $<$<CONFIG:Release>:-g0>
)

add_dependencies(vis triangle_spirv)