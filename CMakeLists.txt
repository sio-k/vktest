cmake_minimum_required(VERSION 3.1)
set(CMAKE_VERBOSE_MAKEFILE ON)

# largely copied from a previous CMakeLists.txt written by Sebbel back in 2016

project(vktest)
# version number
set(VERSION_MAJOR 0 CACHE STRING "Project major version number.")
set(VERSION_MINOR 0 CACHE STRING "Project minor version number.")
set(VERSION_PATCH 1 CACHE STRING "Project patch version number.")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

# set options to choose from
set(CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES} Debug Release RelWithDebInfo MinSizeRel ASAN profile RelAggressive)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES} )

# compiler and project settings
if(CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: None(CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel" FORCE)
else()
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: None(CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif()

set(CMAKE_C_FLAGS                "${CMAKE_C_FLAGS}                -Wall -Wextra -Wpedantic -fno-strict-aliasing")
set(CMAKE_C_FLAGS_DEBUG          "${CMAKE_C_FLAGS_DEBUG}          -O0 -g3 -pg")
set(CMAKE_C_FLAGS_RELEASE        "${CMAKE_C_FLAGS_RELEASE}        -Ofast -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -O2 -g3")
set(CMAKE_C_FLAGS_MINSIZEREL     "${CMAKE_C_FLAGS_MINSIZEREL}     -Os -DNDEBUG")
set(CMAKE_C_FLAGS_ASAN           "${CMAKE_C_FLAGS_ASAN}           -O0 -fsanitize=address")
set(CMAKE_C_FLAGS_RELAGGRESSIVE  "${CMAKE_C_FLAGS_RELAGGRESSIVE}  -Ofast -Werror -DNDEBUG")

set(CMAKE_LD_FLAGS_DEBUG           "${CMAKE_LD_FLAGS_DEBUG}           -pg -fsanitize=address")
set(CMAKE_LD_FLAGS_RELEASE         "${CMAKE_LD_FLAGS_RELEASE}         -fdata-sections -ffunction-sections -Wl,--gc-sections")
set(CMAKE_LD_FLAGS_RELWITHDEBINFO  "${CMAKE_LD_FLAGS_RELWITHDEBINFO}  -fdata-sections -ffunction-sections -Wl,--gc-sections -fsanitize=address ")
set(CMAKE_LD_FLAGS_MINSIZEREL      "${CMAKE_LD_FLAGS_MINSIZEREL}      -fdata-sections -ffunction-sections -Wl,--gc-sections")
set(CMAKE_LD_FLAGS_ASAN            "${CMAKE_LD_FLAGS_ASAN}            -fsanitize=address")
set(CMAKE_LD_FLAGS_RELAGGRESSIVE   "${CMAKE_LD_FLAGS_RELAGGRESSIVE}   -fdata-sections -ffunction-sections -Wl,--gc-sections")

# project files
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

file(GLOB SOURCES_MAIN
    "${SOURCE_DIR}/*.c"
)
file(GLOB INCLUDES_MAIN
    "${SOURCE_DIR}/*.h"
)

# set target
add_executable(${PROJECT_NAME} ${SOURCES_MAIN})

# set project dependencies
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FindPkgConfig)

pkg_search_module(SDL2 REQUIRED sdl2>=2.0.0)
pkg_search_module(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
pkg_search_module(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS} ${SDL2TTF_INCLUDE_DIRS})
# TODO: find out how CMake can find Vulkan by itself
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES} ${SDL2TTF_LIBRARIES} xcb X11 X11-xcb vulkan dl m)
