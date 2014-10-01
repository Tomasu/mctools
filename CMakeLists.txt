cmake_minimum_required(VERSION 2.8)
project(mctools CXX)

include(cmake/Common.cmake)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
   add_definitions(-DDEBUG -O0 -ggdb3)
else()
   add_definitions(-O2 -s -march=native)
endif()

add_definitions(-Wextra -Wall -pipe -std=gnu++11)

# tell CMake to search first in the cmake subdirectory for FIND_PACKAGE() or INCLUDE()
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)

add_subdirectory(qnbt)
add_subdirectory(misc)
add_subdirectory(mapviewer)
add_subdirectory(trimmap)
#add_subdirectory(procmap)
