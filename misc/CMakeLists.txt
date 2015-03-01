find_package(LibNBT REQUIRED)
find_package(LibMCMap REQUIRED)
find_package(LibIOAccess REQUIRED)

include_directories(${LIBMCMAP_INCLUDE_DIRS} ${LIBNBT_INCLUDE_DIRS} ${LIBIOACCESS_INCLUDE_DIRS})

add_executable(loadsave loadsave.cpp)
target_link_libraries(loadsave ${LIBMCMAP_LIBRARIES} ${LIBNBT_LIBRARIES} ${LIBIOACCESS_LIBRARIES})
add_dependencies(loadsave mcmap ioaccess)

add_executable(dump dump.cpp)
target_link_libraries(dump ${LIBNBT_LIBRARIES})
add_dependencies(dump nbt)