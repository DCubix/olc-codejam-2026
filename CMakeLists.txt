cmake_minimum_required(VERSION 3.15)
project(VolatileGame)
 
include(FetchContent)

# Force FetchContent to always check for updates
set(FETCHCONTENT_UPDATES_DISCONNECTED OFF)

# Fetch olcPixelGameEngine3.h
FetchContent_Declare(
    olcPixelGameEngine3
    URL "https://raw.githubusercontent.com/OneLoneCoder/olcPixelGameEngine3/develop/olcPixelGameEngine3.h"
    DOWNLOAD_NO_EXTRACT ON
    SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/olcpixelgameengine3-src"
)
FetchContent_MakeAvailable(olcPixelGameEngine3)

# Fetch the olcUTIL3 utilities.
foreach(UTIL Animate2D Camera2D GameMode Geometry2D QuadTree)
    FetchContent_Declare(
        olcUTIL3_${UTIL}
        URL "https://raw.githubusercontent.com/OneLoneCoder/olcPixelGameEngine3/develop/utilities/olcUTIL3_${UTIL}.h"
        DOWNLOAD_NO_EXTRACT ON
        SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/olcpixelgameengine3-src/utilities"
    )
    FetchContent_MakeAvailable(olcUTIL3_${UTIL})
endforeach()

# Add them to the include paths
include_directories(
    "${CMAKE_BINARY_DIR}/_deps/olcpixelgameengine3-src"
)

# Wrangle Source files
file(
    GLOB_RECURSE SOURCES
    src/*.c
    src/*.cpp
    src/*.h
    src/*.hpp
)

# Non-compiled files, listed only so QtCreator shows them in the project tree
file(GLOB_RECURSE EXTRA_FILES assets/* *.html)
list(FILTER EXTRA_FILES EXCLUDE REGEX "/build/")
list(APPEND SOURCES ${EXTRA_FILES})

# Include helper scripts
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Helpers.cmake)

# platform detection and preparation
pge3_prepare_platforms()

# add_executable with all of the pge3 batteries includes
pge3_add_program(${CMAKE_PROJECT_NAME} ${SOURCES})

# Web build: itch.io expects index.html at the root of the uploaded zip
if(EMSCRIPTEN)
    set(SHELL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html)
    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME index LINK_DEPENDS ${SHELL_FILE})
    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE --shell-file ${SHELL_FILE})
endif()

# copy/manage assets directory
pge3_handle_assets(${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/assets)