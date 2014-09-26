project(trimmap CXX)

find_package(LibMCMap REQUIRED)
find_package(LibNBT REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${LIBNBT_INCLUDE_DIRS} ${LIBMCMAP_INCLUDE_DIRS})

add_executable(trimmap
	procmap.cpp
	BitMap.cpp
	clean.cpp
	process.cpp
	widen.cpp
	worker.cpp
)

target_link_libraries(trimmap ${LIBMCMAP_LIBRARIES} ${LIBNBT_LIBRARIES} -lpthread -lm)
