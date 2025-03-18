cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

project(Pong CXX)

set(CMAKE_CXX_MODULE_STD 1)

option(BUILD_PRE_EXAMPLES "Build the pre examples" ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(glew CONFIG REQUIRED)

function(target_add_extra_warnings)
    set(options PUBLIC PRIVATE INTERFACE)
    set(oneValueArgs TARGET)
    set(multiValueArgs "")

    # Parse the arguments
    cmake_parse_arguments(PARSE_ARGV 0 EXTRA_WARNING_ARGS
            "${options}" "${oneValueArgs}" "${multiValueArgs}"
    )

    # Ensure TARGET is specified
    if (NOT EXTRA_WARNING_ARGS_TARGET)
        message(FATAL_ERROR "TARGET argument is required!")
    endif ()

    # Determine the visibility. Default to PUBLIC if none is provided.
    set(visibility "PUBLIC")  # Default value
    foreach (option IN LISTS options)
        if (EXTRA_WARNING_ARGS_${option})
            set(visibility ${option})
            break()
        endif ()
    endforeach ()

    # Apply compile options to the given target with the resolved visibility
    # Add compile options to the target with the specified visibility
    target_compile_options(${target} ${visibility}
            $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wnon-virtual-dtor>
    )
endfunction()

## extra warnings
#function(target_add_extra_warnings target visibility)
#
#    # Ensure TARGET is specified
#    if (NOT target)
#        message(FATAL_ERROR "TARGET argument is required!")
#    endif ()
#
#    if (NOT visibility)
#        set(visibility PUBLIC)
#    endif ()
#
#    set(valid_visibilities PUBLIC PRIVATE INTERFACE)
#    if (NOT ${visibility} IN_LIST valid_visibilities)
#        message(FATAL_ERROR "${visibility} not PUBLIC, PRIVATE or INTERFACE")
#    endif ()
#
#    target_compile_options(${target} ${visibility}
#            $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wnon-virtual-dtor>
#    )
#endfunction()

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

add_subdirectory(vis)
add_subdirectory(pong)
