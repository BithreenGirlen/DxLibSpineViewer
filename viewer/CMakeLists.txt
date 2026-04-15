cmake_minimum_required(VERSION 3.20)
project(DxLibSpineViewer)

set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../deps")
set(DXLIB_DIR "${DEPS_DIR}/dxlib")
set(IMGUI_DOCKING_DIR "${DEPS_DIR}/imgui-docking")

set(VIEWER_IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dxlib-imgui")
set(VIEWER_NATIVE_UI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/native-ui")
set(SPINE_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../runtime")

set(OUTPUT_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set(OUTPUT_ARCHIVE_DIR "${CMAKE_BINARY_DIR}/lib")

set(TARGET_NAME "DxLibSpineViewer")

add_executable(${TARGET_NAME} WIN32
	"${IMGUI_DOCKING_DIR}/imconfig.h"
	"${IMGUI_DOCKING_DIR}/imgui.cpp" "${IMGUI_DOCKING_DIR}/imgui.h"
	"${IMGUI_DOCKING_DIR}/imgui_demo.cpp"
	"${IMGUI_DOCKING_DIR}/imgui_draw.cpp"
	"${IMGUI_DOCKING_DIR}/imgui_internal.h"
	"${IMGUI_DOCKING_DIR}/imgui_tables.cpp"
	"${IMGUI_DOCKING_DIR}/imgui_widgets.cpp"
	"${IMGUI_DOCKING_DIR}/backends/imgui_impl_win32.cpp" "${IMGUI_DOCKING_DIR}/backends/imgui_impl_win32.h"
	"${IMGUI_DOCKING_DIR}/backends/imgui_impl_dx11.cpp" "${IMGUI_DOCKING_DIR}/backends/imgui_impl_dx11.h"
	"${IMGUI_DOCKING_DIR}/backends/imgui_impl_dx9.cpp" "${IMGUI_DOCKING_DIR}/backends/imgui_impl_dx9.h"
	"dpiAwareness.manifest"
	"cpu.cpp" "cpu.h"
	"dxlib_handle.h"
	"dxlib_image_encoder.cpp" "dxlib_image_encoder.h"
	"dxlib_init.cpp" "dxlib_init.h"
	"dxlib_map.cpp" "dxlib_map.h"
	"dxlib_recorder.cpp" "dxlib_recorder.h"
	"dxlib_render_target_scope.cpp" "dxlib_render_target_scope.h"
	"json_minimal.cpp" "json_minimal.h"
	"main.cpp"
	"main_window.cpp" "main_window.h"
	"mf_video_encoder.cpp" "mf_video_encoder.h"
	"spine_player_dynamic.cpp" "spine_player_dynamic.h"
	"text_utility.h"
	"win_clock.cpp" "win_clock.h"
	"win_dialogue.cpp" "win_dialogue.h"
	"win_filesystem.cpp" "win_filesystem.h"
	"win_font.cpp" "win_font.h"
	"win_image.cpp" "win_image.h"
	"win_text.cpp" "win_text.h"
	"${VIEWER_IMGUI_DIR}/dxlib_imgui.cpp" "${VIEWER_IMGUI_DIR}/dxlib_imgui.h"
	"${VIEWER_IMGUI_DIR}/spine_tool_dialogue.cpp" "${VIEWER_IMGUI_DIR}/spine_tool_dialogue.h"
	"${VIEWER_NATIVE_UI_DIR}/dialogue_controls.cpp" "${VIEWER_NATIVE_UI_DIR}/dialogue_controls.h"
	"${VIEWER_NATIVE_UI_DIR}/dialogue_template.cpp" "${VIEWER_NATIVE_UI_DIR}/dialogue_template.h"
	"${VIEWER_NATIVE_UI_DIR}/font_setting_dialogue.cpp" "${VIEWER_NATIVE_UI_DIR}/font_setting_dialogue.h"
	"${VIEWER_NATIVE_UI_DIR}/spine_setting_dialogue.cpp" "${VIEWER_NATIVE_UI_DIR}/spine_setting_dialogue.h"
	"${VIEWER_NATIVE_UI_DIR}/window_menu.cpp" "${VIEWER_NATIVE_UI_DIR}/window_menu.h"
	"${SPINE_RUNTIME_DIR}/spine_player_dll.h"
	"${SPINE_RUNTIME_DIR}/dxlib_spine_dll.cpp" "${SPINE_RUNTIME_DIR}/dxlib_spine_dll.h"
	"${SPINE_RUNTIME_DIR}/spine_file_verifier.cpp" "${SPINE_RUNTIME_DIR}/spine_file_verifier.h"
)

set(TARGET_FILENAME "DxLibSpineViewer")

set_target_properties(${TARGET_NAME} PROPERTIES
	OUTPUT_NAME "${TARGET_FILENAME}"
	RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_BIN_DIR}"
	ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_ARCHIVE_DIR}"
)

target_include_directories(${TARGET_NAME} PRIVATE
	${DXLIB_DIR}
	${IMGUI_DOCKING_DIR}
)

target_link_directories(${TARGET_NAME} PRIVATE ${DXLIB_DIR})

target_compile_definitions(${TARGET_NAME} PRIVATE UNICODE _UNICODE)

target_precompile_headers(${TARGET_NAME} PRIVATE
	<string>
	<vector>
	<memory>
	<algorithm>
	<unordered_map>
)

if(MSVC)
	target_compile_definitions(${TARGET_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS)
	target_compile_options(${TARGET_NAME} PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/d1trimfile:${CMAKE_SOURCE_DIR}\\>")
	target_link_options(${TARGET_NAME} PRIVATE "/pdbaltpath:%_PDB%")
endif()
