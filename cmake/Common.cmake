function(add_copy_commands src dest destfilesvar)
	set(destfiles)
	foreach(basename ${ARGN})
		list(APPEND destfiles "${dest}/${basename}")
		add_custom_command(
			OUTPUT  "${dest}/${basename}"
			DEPENDS "${src}/${basename}"
			COMMAND "${CMAKE_COMMAND}" -E copy
						"${src}/${basename}" "${dest}/${basename}"
			)
	endforeach()
	set(${destfilesvar} "${destfiles}" PARENT_SCOPE)
endfunction()

# Recreate data directory for out-of-source builds.
# Note: a symlink is unsafe as make clean will delete the contents
# of the pointed-to directory.
#
# Files are only copied if they don't are inside a .svn folder so we
# won't end up with read-only .svn folders in the build folder.
function(copy_data_dir_to_build target src dest)
	if(src STREQUAL dest)
		return()
	endif()

	file(GLOB_RECURSE files RELATIVE "${src}" "${src}/*")
	add_copy_commands("${src}" "${dest}" destfiles "${files}")
	add_custom_target(${target} ALL DEPENDS ${destfiles})
endfunction(copy_data_dir_to_build)

function(copy_to_build file_name)
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file_name}
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${file_name} ${CMAKE_CURRENT_BINARY_DIR}/${file_name}
		MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${file_name})
endfunction()