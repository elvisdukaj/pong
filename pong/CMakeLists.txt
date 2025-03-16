cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

add_executable(pong)

target_sources(pong
        PUBLIC main.cpp
        PUBLIC FILE_SET CXX_MODULES FILES game.cpp)

target_link_libraries(pong PRIVATE vis::vis)