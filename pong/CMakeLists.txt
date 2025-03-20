cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

find_package(sml CONFIG REQUIRED)

add_executable(pong)

target_sources(pong
        PUBLIC main.cpp
        PUBLIC FILE_SET CXX_MODULES
        FILES
        game.cpp
        events.cpp
        pong.cpp
)

target_link_libraries(pong PRIVATE vis::vis sml::sml)
target_add_extra_warnings(pong PUBLIC)
target_set_warnings_as_error(pong)