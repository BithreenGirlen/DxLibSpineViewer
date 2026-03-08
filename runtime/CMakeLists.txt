cmake_minimum_required(VERSION 3.20)
project(DxLibSpinePlayer-dll)

set(DXLIB_SPINE_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../deps")
set(DXLIB_DIR "${DEPS_DIR}/dxlib")

set(OUTPUT_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set(OUTPUT_ARCHIVE_DIR "${CMAKE_BINARY_DIR}/lib")

set(SPINE_VERSIONS_C   2.1 3.5 3.6 3.7)
set(SPINE_VERSIONS_CPP 3.8 4.0 4.1 4.2)

function(add_spine_target RUNTIME_LANGUAGE RUNTIME_VERSION)
	# Handle the version format difference between directory (X.Y) and dll (XY)
	string(REPLACE "." "" RUNTIME_VERSION_WITHOUT_DOT "${RUNTIME_VERSION}")
	set(SPINE_GENERIC_RUNTIME_DIR "${DEPS_DIR}/spine-${RUNTIME_LANGUAGE}-${RUNTIME_VERSION}")

	if(RUNTIME_LANGUAGE STREQUAL "C")
		set(TARGET_NAME "SpinePlayerC_${RUNTIME_VERSION_WITHOUT_DOT}")
		set(TARGETT_FILENAME "DxLibSpinePlayerC${RUNTIME_VERSION_WITHOUT_DOT}")
		set(DXLIB_SPINE_SRC_DIR "${DXLIB_SPINE_RUNTIME_DIR}/DxLibSpineC")
		set(SPINE_GENERIC_RUNTIME_SRC_EXT "c")
	else()
		set(TARGET_NAME "SpinePlayerCpp_${RUNTIME_VERSION_WITHOUT_DOT}")
		set(TARGETT_FILENAME "DxLibSpinePlayer${RUNTIME_VERSION_WITHOUT_DOT}")
		set(DXLIB_SPINE_SRC_DIR "${DXLIB_SPINE_RUNTIME_DIR}/DxLibSpineCpp")
		set(SPINE_GENERIC_RUNTIME_SRC_EXT "cpp")
	endif()

	add_library(
		${TARGET_NAME} SHARED
		${DXLIB_SPINE_RUNTIME_DIR}/spine_player_dll.cpp
		${DXLIB_SPINE_RUNTIME_DIR}/spine_player_dll.h
		${DXLIB_SPINE_RUNTIME_DIR}/dxlib_spine_dll.cpp
		${DXLIB_SPINE_RUNTIME_DIR}/dxlib_spine_dll.h
	)

	set_target_properties(${TARGET_NAME} PROPERTIES
		OUTPUT_NAME "${TARGETT_FILENAME}"
		RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}"
		ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_ARCHIVE_DIR}"
	)

	# Spine 2.1 has different attachment types from later versions.
	if(RUNTIME_LANGUAGE STREQUAL "C" AND RUNTIME_VERSION EQUAL 2.1)
		file(GLOB DXLIB_SRC "${DXLIB_SPINE_SRC_DIR}/*.cpp")
		list(REMOVE_ITEM DXLIB_SRC "${DXLIB_SPINE_SRC_DIR}/dxlib_spine_c.cpp")
		target_sources(${TARGET_NAME} PRIVATE
			${DXLIB_SPINE_RUNTIME_DIR}/DxLibSpineC/Spine21/dxlib_spine_c_21.cpp
			${DXLIB_SRC}
		)
	else()
		file(GLOB DXLIB_SRC "${DXLIB_SPINE_SRC_DIR}/*.cpp")
		target_sources(${TARGET_NAME} PRIVATE ${DXLIB_SRC})
	endif()

	# Spine generic runtime sources
	if(SPINE_GENERIC_RUNTIME_DIR)
		file(GLOB SPINE_SRC "${SPINE_GENERIC_RUNTIME_DIR}/src/spine/*.${SPINE_GENERIC_RUNTIME_SRC_EXT}")
		target_sources(${TARGET_NAME} PRIVATE ${SPINE_SRC})
	endif()

	target_include_directories(${TARGET_NAME} PRIVATE
		${DXLIB_DIR}
		$<$<BOOL:${SPINE_GENERIC_RUNTIME_DIR}>:${SPINE_GENERIC_RUNTIME_DIR}/include>
		${DXLIB_SPINE_SRC_DIR}
	)

	target_compile_definitions(${TARGET_NAME} PRIVATE SPINE_${RUNTIME_VERSION_WITHOUT_DOT} UNICODE _UNICODE)

	target_compile_definitions(${TARGET_NAME} PRIVATE SPINE_${RUNTIME_VERSION_WITHOUT_DOT} SPINE_RUNTIME_DLL_BUILD)
	target_compile_definitions(${TARGET_NAME} PRIVATE SPINE_${RUNTIME_VERSION_WITHOUT_DOT} DX_MAKE)
	if(RUNTIME_LANGUAGE STREQUAL "C")
		target_compile_definitions(${TARGET_NAME} PRIVATE SPINE_${RUNTIME_VERSION_WITHOUT_DOT} SPINE_C)
	else()
		target_compile_definitions(${TARGET_NAME} PRIVATE SPINE_${RUNTIME_VERSION_WITHOUT_DOT} SPINE_CPP)
	endif()

	if(MSVC)
		target_compile_definitions(${TARGET_NAME} PRIVATE SPINE_${RUNTIME_VERSION_WITHOUT_DOT} -D_CRT_SECURE_NO_WARNINGS)
		target_compile_options(${TARGET_NAME} PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/d1trimfile:${CMAKE_SOURCE_DIR}\\>")
		target_link_options(${TARGET_NAME} PRIVATE "/pdbaltpath:%_PDB%")
	endif()

endfunction()

foreach(VER IN LISTS SPINE_VERSIONS_C)
	add_spine_target("C" ${VER})
endforeach()

foreach(VER IN LISTS SPINE_VERSIONS_CPP)
	add_spine_target("Cpp" ${VER})
endforeach()
