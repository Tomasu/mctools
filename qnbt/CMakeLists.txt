project(qnbt CXX)

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

find_package(LibNBT REQUIRED)

QT4_WRAP_CPP(MOCSRC MainWindow.h NBTModel.h NBTItemDelegate.h)
 
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${LIBNBT_INCLUDE_DIRS})

add_definitions(${MAP_CFLAGS})

add_executable(qnbt 
	#InventoryWindow.cpp
	#InventoryModel.cpp
	main.cpp
	MainWindow.cpp
	NBTModel.cpp
	NBTItemDelegate.cpp
	${MOCSRC}
)

add_dependencies(qnbt nbt)
target_link_libraries(qnbt ${QT_LIBRARIES} ${LIBNBT_LIBRARIES})
