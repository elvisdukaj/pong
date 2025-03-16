cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

project(Pong CXX)

set(CMAKE_CXX_MODULE_STD 1)

option(BUILD_PRE_EXAMPLES "Build the pre examples" ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(glew CONFIG REQUIRED)

add_subdirectory(vis)
add_subdirectory(pong)
