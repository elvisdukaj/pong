cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

project(Pong CXX)

set(CMAKE_CXX_MODULE_STD 1)

option(BUILD_PRE_EXAMPLES "Build the pre examples" ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(glew CONFIG REQUIRED)

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

add_subdirectory(vis)
add_subdirectory(pong)
