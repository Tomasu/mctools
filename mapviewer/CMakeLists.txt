project(viewer CXX)

include(FindPkgConfig)

set(VIEWER_HEADERS
	include/tgetopt.h
	include/Renderer.h
	include/BlockData.h
	include/ChunkData.h
	include/Resource/Atlas.h
	include/Resource/AtlasSheet.h
	include/Resource/AtlasBitmap.h
	include/Resource/Bitmap.h
	include/Resource/Manager.h
)

set(VIEWER_SOURCE 
	src/tgetopt.cpp 
	src/main.cpp
	src/Renderer.cpp
	src/ChunkData.cpp
	src/BlockData.cpp
	src/Resource/Manager.cpp
	src/Resource/Resource.cpp
	src/Resource/Atlas.cpp
	src/Resource/AtlasSheet.cpp
	src/Resource/AtlasBitmap.cpp
	shaders/default.vtx
	shaders/default.pxl
)

if(PKG_CONFIG_FOUND)
	message(STATUS "checking for allegro")
	pkg_check_modules(ALLEGRO REQUIRED allegro-debug-5)
	pkg_check_modules(ALLEGRO_MAIN REQUIRED allegro_main-debug-5)
	pkg_check_modules(ALLEGRO_IMAGE REQUIRED allegro_image-debug-5)
	pkg_check_modules(ALLEGRO_PRIMITIVES REQUIRED allegro_primitives-debug-5)
	
	if(ALLEGRO_FOUND AND ALLEGRO_PRIMITIVES_FOUND AND ALLEGRO_IMAGE_FOUND)
		message(STATUS "Allegro ${ALLEGRO_VERSION} was found, enable renderer.")
		add_definitions(${ALLEGRO_CFLAGS})
		message(STATUS "cflags ${ALLEGRO_CFLAGS} libs ${ALLEGRO_LIBRARIES} ${ALLEGRO_MAIN_LIBRARIES} ${ALLEGRO_IMAGE_LIBRARIES} ${ALLEGRO_PRIMITIVES_LIBRARIES}")
		find_package(LibMCMap REQUIRED)
		find_package(LibNBT REQUIRED)
		
		copy_to_build(shaders/default.vtx)
		copy_to_build(shaders/default.pxl)
		
		message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
		message(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
		
		include_directories(${PROJECT_SOURCE_DIR}/include ${LIBMCMAP_INCLUDE_DIRS} ${LIBNBT_INCLUDE_DIRS})
		add_executable(viewer ${VIEWER_SOURCE} ${VIEWER_HEADERS})
		target_link_libraries(viewer ${ALLEGRO_LIBRARIES} ${LIBMCMAP_LIBRARIES} ${LIBNBT_LIBRARIES} GL ${ALLEGRO_PRIMITIVES_LIBRARIES} ${ALLEGRO_IMAGE_LIBRARIES})

		if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
			message(STATUS "copy assets")
			copy_data_dir_to_build(copy_viewer_assest
				"${CMAKE_SOURCE_DIR}/assets"
				"${CMAKE_CURRENT_BINARY_DIR}/assets"
			)
		endif()
		
	else()
		message(STATUS "Allegro library not found, disabling renderer")
	endif()
else()
	message(STATUS "pkg-config not found, disabling renderer")
endif()

