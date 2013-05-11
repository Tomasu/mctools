project(procmap CXX)

find_package(Lua52 REQUIRED)

include_directories(${LUA_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(${MAP_CFLAGS})

add_executable(procmap
	procmap.cpp
	BitMap.cpp
	clean.cpp
	process.cpp
	widen.cpp
	worker.cpp
)

add_dependencies(procmap mcmap)
target_link_libraries(procmap ${MAP_LIBS} ${LUA_LIBRARIES} -lpthread -lm)
