find_package(LibNBT REQUIRED)
find_package(LibMCMap REQUIRED)

include_directories(${LIBMCMAP_INCLUDE_DIRS} ${LIBNBT_INCLUDE_DIRS})

add_executable(loadsave loadsave.cpp)
target_link_libraries(loadsave ${LIBMCMAP_LIBRARIES} ${LIBNBT_LIBRARIES})
add_dependencies(loadsave mcmap)
