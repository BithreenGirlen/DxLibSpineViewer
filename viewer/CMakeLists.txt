cmake_minimum_required(VERSION 3.20)
project(DxLibSpineViewer)

set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../deps")
set(DXLIB_DIR "${DEPS_DIR}/dxlib")
set(IMGUI_DOCKING_DIR "${DEPS_DIR}/imgui-docking")

set(OUTPUT_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set(OUTPUT_ARCHIVE_DIR "${CMAKE_BINARY_DIR}/lib")

set(TARGET_NAME "DxLibSpineViewer")

file(GLOB DEAR_IMGUI_SRC "${IMGUI_DOCKING_DIR}/*.cpp" "${IMGUI_DOCKING_DIR}/*.h")

file(GLOB VIEWER_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB VIEWER_IMGUI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/dxlib-imgui/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/dxlib-imgui/*.h")
file(GLOB VIEWER_NATIVE_UI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/native-ui/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/native-ui/*.h")

add_executable(${TARGET_NAME} WIN32
	${DEAR_IMGUI_SRC}
	"${IMGUI_DOCKING_DIR}/backends/imgui_impl_win32.cpp" "${IMGUI_DOCKING_DIR}/backends/imgui_impl_win32.h"
	"${IMGUI_DOCKING_DIR}/backends/imgui_impl_dx11.cpp" "${IMGUI_DOCKING_DIR}/backends/imgui_impl_dx11.h"
	"${IMGUI_DOCKING_DIR}/backends/imgui_impl_dx9.cpp" "${IMGUI_DOCKING_DIR}/backends/imgui_impl_dx9.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/dpiAwareness.manifest"
	${VIEWER_SRC}
	${VIEWER_IMGUI_SRC}
	${VIEWER_NATIVE_UI_SRC}
	"../runtime/spine_player_dll.h"
	"../runtime/dxlib_spine_dll.cpp"
	"../runtime/dxlib_spine_dll.h"
	"../runtime/spine_file_verifier.cpp"
	"../runtime/spine_file_verifier.h"
)

set(TARGETT_FILENAME "DxLibSpineViewer")

set_target_properties(${TARGET_NAME} PROPERTIES
	OUTPUT_NAME "${TARGETT_FILENAME}"
	RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}"
	ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_ARCHIVE_DIR}"
)

target_include_directories(${TARGET_NAME} PRIVATE
	${DXLIB_DIR}
	${IMGUI_DOCKING_DIR}
)

target_link_directories(${TARGET_NAME} PRIVATE ${DXLIB_DIR})

target_compile_definitions(${TARGET_NAME} PRIVATE UNICODE _UNICODE)

if(MSVC)
	target_compile_definitions(${TARGET_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS)
	target_compile_options(${TARGET_NAME} PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/d1trimfile:${CMAKE_SOURCE_DIR}\\>")
	target_link_options(${TARGET_NAME} PRIVATE "/pdbaltpath:%_PDB%")
endif()
