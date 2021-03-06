cmake_minimum_required(VERSION 2.6)
project(grender)

# Enable debug symbols by default
if(CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE Debug)
endif()
# (you can also set it on the command line: -D CMAKE_BUILD_TYPE=Release)

# Set version information in a config.h file
set(grender_VERSION_MAJOR 0)
set(grender_VERSION_MINOR 1)
set(grender_VERSION_PATCH 0)
configure_file(
  "${PROJECT_SOURCE_DIR}/src/config.h.in"
  "${PROJECT_BINARY_DIR}/config/config.h"
)
include_directories("${PROJECT_BINARY_DIR}/config")

# Detect and add SDL
Find_Package(SDL REQUIRED)

if(NOT SDL_FOUND)
   message ( FATAL_ERROR "SDL not found!" )
endif(NOT SDL_FOUND)

link_libraries (
   m
   ${SDL_LIBRARY}
   SDLmain
)

# Define sources and executable
set(EXECUTABLE_NAME "grender")
set(SOURCES
    src/Context.c
    src/Draw.c
    src/MatrixStack.c
    src/Shader.c
    src/Vertex.c
    src/Raster.c
    src/main.c
)
add_executable(
    ${EXECUTABLE_NAME}
    WIN32
    MACOSX_BUNDLE
    ${SOURCES}
)
add_custom_command(
    TARGET grender
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/data 
                                           ${PROJECT_BINARY_DIR}/data
)

set(CMAKE_CXX_FLAGS "-O2 -Wall -Werror ${CMAKE_CXX_FLAGS}")

# Install target
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)

# CPack packaging
include(InstallRequiredSystemLibraries)
#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR "${grender_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${grender_VERSION_MINOR}")
include(CPack)
