project(viewer CXX)

function(copy_to_build file_name)
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file_name}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${file_name} ${CMAKE_CURRENT_BINARY_DIR}/${file_name}
		MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${file_name})
endfunction()

include(FindPkgConfig)

if(PKG_CONFIG_FOUND)
	message(STATUS "checking for allegro")
	pkg_check_modules(ALLEGRO REQUIRED allegro-5)
	pkg_check_modules(ALLEGRO_MAIN REQUIRED allegro_main-5)
	pkg_check_modules(ALLEGRO_IMAGE REQUIRED allegro_image-5)
	pkg_check_modules(ALLEGRO_PRIMITIVES REQUIRED allegro_primitives-5)
	
	if(ALLEGRO_FOUND)
		message(STATUS "Allegro ${ALLEGRO_VERSION} was found, enable renderer.")
		add_definitions(${ALLEGRO_CFLAGS})

		find_package(LibMCMap REQUIRED)
		find_package(LibNBT REQUIRED)
		
		copy_to_build(shaders/default.vtx)
		copy_to_build(shaders/default.pxl)
		include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${LIBMCMAP_INCLUDE_DIRS} ${LIBNBT_INCLUDE_DIRS})
		add_executable(viewer src/main.cpp src/Renderer.cpp src/ChunkData.cpp src/BlockData.cpp src/tgetopt.cpp shaders/default.vtx shaders/default.pxl)
		target_link_libraries(viewer ${ALLEGRO_LIBRARIES} ${LIBMCMAP_LIBRARIES} ${LIBNBT_LIBRARIES} GL ${ALLEGRO_PRIMITIVES_LIBRARIES})

	else()
		message(STATUS "Allegro library not found, disabling renderer")
	endif()
else()
	message(STATUS "pkg-config not found, disabling renderer")
endif()

