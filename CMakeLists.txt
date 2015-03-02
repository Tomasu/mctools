cmake_minimum_required(VERSION 2.8)
project(mctools CXX)

include(cmake/Common.cmake)

option(WANT_QNBT "build qnbt" ON)
option(WANT_MISC "build misc programs" OFF)
option(WANT_TRIMMAP "build trimmap" ON)
option(WANT_PROCMAP "build procmap" ON)
option(WANT_MAPVIEWER "build mapviewer" ON)

option(SHARED "use shared libs" ON)
if(NOT SHARED)
	IF(WIN32)
		SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
	ELSE(WIN32)
		SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
	ENDIF(WIN32)
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
   add_definitions(-DDEBUG -O0 -ggdb3)
else()
   add_definitions(-O2 -s -march=native)
endif()

add_definitions(-Wextra -Wall -pipe)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

# tell CMake to search first in the cmake subdirectory for FIND_PACKAGE() or INCLUDE()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/)

if(WANT_QNBT)
	add_subdirectory(qnbt)
endif()

if(WANT_MISC)
	add_subdirectory(misc)
endif()

if(WANT_MAPVIEWER)
	add_subdirectory(mapviewer)
endif()

if(WANT_TRIMMAP)
	add_subdirectory(trimmap)
endif()

#add_subdirectory(procmap)
