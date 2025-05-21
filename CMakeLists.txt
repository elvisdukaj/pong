cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

project(Pong CXX)

set(CMAKE_CXX_MODULE_STD 1)

option(BUILD_PRE_EXAMPLES "Build the pre examples" ON)
option(WITH_TIDY "Enable clang-tidy" OFF)
option(WITH_ADDRESS_SANITIZER "Enable address sanitizer" ON)

set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/resources)
set(RESOURCE_SHADER_DIR ${CMAKE_SOURCE_DIR}/resources/shaders)

if (WITH_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-p;${CMAKE_BINARY_DIR}")
endif ()

function(target_add_extra_warnings target visibility)
    # Ensure TARGET is specified
    if (NOT target)
        message(FATAL_ERROR "TARGET argument is required!")
    endif ()

    # Set default visibility if none is provided
    if (NOT visibility)
        set(visibility PUBLIC)
    endif ()

    # Define valid visibilities
    set(valid_visibilities PUBLIC PRIVATE INTERFACE)

    # Check if the provided visibility is valid
    list(FIND valid_visibilities ${visibility} index) # Search for visibility in the list
    if (index EQUAL -1)
        message(FATAL_ERROR "${visibility} is not PUBLIC, PRIVATE, or INTERFACE")
    endif ()

    # Add compile options to the target with the specified visibility
    target_compile_options(${target} ${visibility}
            $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wnon-virtual-dtor>
    )
endfunction()


function(target_set_warnings_as_error target)
    if (NOT target)
        message(FATAL_ERROR "TARGET argument is required!")
    endif ()

    target_compile_options(${target} PRIVATE
            $<$<CXX_COMPILER_ID:Clang>:-Werror>
    )
endfunction()

# Hack for CLion on MacOS to make import std intellisens work
if (BREW_LLVM_LIBCPP)
    message(WARNING "Using hack to make CLion intellisense works with import std. The std.compat.cppm is located at ${BREW_LLVM_LIBCPP}")
    add_library(unused_std_target STATIC)
    target_sources(unused_std_target
            PRIVATE
            FILE_SET CXX_MODULES
            BASE_DIRS ${BREW_LLVM_LIBCPP}
            FILES "${BREW_LLVM_LIBCPP}/std.compat.cppm" "${BREW_LLVM_LIBCPP}/std.cppm"
    )
endif ()
if (LINUX_LLVM_LIBCPP)
    message(WARNING "Using hack to make CLion intellisense works with import std. The std.compat.cppm is located at ${LINUX_LLVM_LIBCPP}")
    add_library(unused_std_target STATIC)
    target_sources(unused_std_target
            PRIVATE
            FILE_SET CXX_MODULES
            BASE_DIRS ${LINUX_LLVM_LIBCPP}
            FILES "${LINUX_LLVM_LIBCPP}/std.compat.cppm" "${LINUX_LLVM_LIBCPP}/std.cppm"
    )
endif ()

add_subdirectory(vis)
add_subdirectory(pong)
