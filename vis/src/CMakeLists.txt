cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

add_library(vis)
add_library(vis::vis ALIAS vis)

target_link_libraries(vis PUBLIC vis_obj::vis_obj)
