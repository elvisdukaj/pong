cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

add_library(vis_obj OBJECT)
add_library(vis_obj::vis_obj ALIAS vis_obj)

target_sources(vis_obj
        PUBLIC FILE_SET CXX_MODULES FILES
        opengl.cpp
        engine.cpp
        vis.cpp
        math.cpp
        entt.cpp
        mesh.cpp
        physic.cpp
)

target_compile_definitions(vis_obj PUBLIC "SDL_MAIN_USE_CALLBACKS=1" ENTT_STANDARD_CPP)
target_link_libraries(vis_obj PUBLIC SDL3::SDL3 EnTT::EnTT glm::glm GLEW::GLEW box2d::box2d)

target_add_extra_warnings(vis_obj PUBLIC)
target_set_warnings_as_error(vis_obj)