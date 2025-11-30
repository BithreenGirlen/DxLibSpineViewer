cmake_minimum_required (VERSION 3.25)
project(DxLibSpineViewer-deps)

set(IMGUI_VERSION "1.92.2-docking")
set(DIR_IMGUI "${CMAKE_CURRENT_LIST_DIR}/imgui-docking")

set(DIR_DXLIB ${CMAKE_CURRENT_LIST_DIR}/dxlib)

set(DIR_TEMP ${CMAKE_CURRENT_BINARY_DIR}/temp)

# Download DxLib.
# --------------------
# The version to be used is the latest, so download from "../temp/DxLibVCTest.zip".
# The static libraries in need are those for Visual Studio x64 MDd or MD.

if (NOT EXISTS "${DIR_DXLIB}")
	set(DXLIB_ARCHIVE_NAME "DxLibVCTest")
	if (NOT EXISTS "${DIR_TEMP}/${DXLIB_ARCHIVE_NAME}.zip")
		file(DOWNLOAD "https://dxlib.xsrv.jp/temp/DxLibVCTest.zip" "${DIR_TEMP}/${DXLIB_ARCHIVE_NAME}.zip" SHOW_PROGRESS)
	endif()

	if (NOT EXISTS "${DIR_TEMP}/${DXLIB_ARCHIVE_NAME}")
		file(ARCHIVE_EXTRACT INPUT "${DIR_TEMP}/${DXLIB_ARCHIVE_NAME}.zip" DESTINATION "${DIR_TEMP}")
	endif()

	file(GLOB DXLIB_LIBS "${DIR_TEMP}/DxLibVCTest/*.lib")
	foreach(FILE IN LISTS DXLIB_LIBS)
		if(FILE MATCHES "_vs2015_x64_MDd.lib" OR FILE MATCHES "_vs2015_x64_MD.lib")
			file(COPY ${FILE} DESTINATION "${DIR_DXLIB}")
		endif()
	endforeach()

	file(GLOB INCLUDE_FILES "${DIR_TEMP}/DxLibVCTest/*.h")
	foreach(FILE IN LISTS INCLUDE_FILES)
		file(COPY ${FILE} DESTINATION "${DIR_DXLIB}")
	endforeach()
endif()

# Download Spine runtime
# --------------------
# The runtimes of interest are spine-c for spine 3.7 and older, and spine-cpp for 3.8 and later.
# Spine 2.1 has no branch, so has to be obtained from 2.1.25 tag.
# Also, this version has different directory depths from later runtimes.

set(
	SPINE_VERSIONS
	"3.5:spine-c-3.5"
	"3.6:spine-c-3.6"
	"3.7:spine-c-3.7"
	"3.8:spine-cpp-3.8"
	"4.0:spine-cpp-4.0"
	"4.1:spine-cpp-4.1"
	"4.2:spine-cpp-4.2"
)

function(donwload_spine_runtime BRANCH_NAME FOLDER_PATH)
	set (SPINE_RUNTIME_URL "https://github.com/EsotericSoftware/spine-runtimes/archive/refs/heads/${BRANCH_NAME}.zip")
	set (SPINE_RUNTIME_FILEPATH "${DIR_TEMP}/spine-runtimes-${BRANCH_NAME}")
	if (NOT EXISTS "${SPINE_RUNTIME_FILEPATH}.zip")
		file(DOWNLOAD "${SPINE_RUNTIME_URL}" "${SPINE_RUNTIME_FILEPATH}.zip" SHOW_PROGRESS)
	endif()

	if (NOT EXISTS "${SPINE_RUNTIME_FILEPATH}")
		file(ARCHIVE_EXTRACT INPUT "${SPINE_RUNTIME_FILEPATH}.zip" DESTINATION "${DIR_TEMP}")
	endif()

	if (FOLDER_PATH MATCHES "spine-c-")
		file(COPY "${DIR_TEMP}/spine-runtimes-${BRANCH_NAME}/spine-c/spine-c/src" DESTINATION "${FOLDER_PATH}")
		file(COPY "${DIR_TEMP}/spine-runtimes-${BRANCH_NAME}/spine-c/spine-c/include" DESTINATION "${FOLDER_PATH}")
	else()
		file(COPY "${DIR_TEMP}/spine-runtimes-${BRANCH_NAME}/spine-cpp/spine-cpp/src" DESTINATION "${FOLDER_PATH}")
		file(COPY "${DIR_TEMP}/spine-runtimes-${BRANCH_NAME}/spine-cpp/spine-cpp/include" DESTINATION "${FOLDER_PATH}")
	endif()
endfunction()

foreach(VERSION IN LISTS SPINE_VERSIONS)
	string(REPLACE ":" ";" VERSIONS ${VERSION})
	list(GET VERSIONS 0 BRANCH_NAME)
	list(GET VERSIONS 1 FOLDER_NAME)

	set(DIR_SPINE "${CMAKE_CURRENT_LIST_DIR}/${FOLDER_NAME}")

	if (NOT EXISTS "${DIR_SPINE}/src")
		donwload_spine_runtime(${BRANCH_NAME} ${DIR_SPINE})
	endif()
endforeach()

# Exceptional procedure for spine 2.1 runtime
set(DIR_SPINE_C_21 ${CMAKE_CURRENT_LIST_DIR}/spine-c-2.1)
if (NOT EXISTS "${DIR_SPINE_C_21}/src")
	set (SPINE_21_RUNTIME_URL "https://github.com/EsotericSoftware/spine-runtimes/archive/refs/tags/2.1.25.zip")
	set (SPINE_21_RUNTIME_FILEPATH "${DIR_TEMP}/spine-runtimes-2.1.25")
	if (NOT EXISTS "${SPINE_21_RUNTIME_FILEPATH}.zip")
		file(DOWNLOAD "${SPINE_21_RUNTIME_URL}" "${SPINE_21_RUNTIME_FILEPATH}.zip" SHOW_PROGRESS)
	endif()

	if (NOT EXISTS "${SPINE_21_RUNTIME_FILEPATH}")
		file(ARCHIVE_EXTRACT INPUT "${SPINE_21_RUNTIME_FILEPATH}.zip" DESTINATION "${DIR_TEMP}")
	endif()
	
	# One depth shorter than later versions.
	file(COPY "${DIR_TEMP}/spine-runtimes-2.1.25/spine-c/src" DESTINATION "${DIR_SPINE_C_21}")
	file(COPY "${DIR_TEMP}/spine-runtimes-2.1.25/spine-c/include" DESTINATION "${DIR_SPINE_C_21}")

	# Add skeleton binary reader and overwrite some of the files for spine 2.1.
	set (SPINE_21_RUNTIME_BINARY_READER_URL "https://github.com/BithreenGirlen/spine-c-2.1.27/archive/refs/heads/main.zip")
	set (SPINE_21_RUNTIME_BINARY_READER_FILENAME "${DIR_TEMP}/spine-c-2.1.27.zip")

	if (NOT EXISTS "${SPINE_21_RUNTIME_BINARY_READER_FILENAME}")
		file(DOWNLOAD "${SPINE_21_RUNTIME_BINARY_READER_URL}" "${SPINE_21_RUNTIME_BINARY_READER_FILENAME}" SHOW_PROGRESS)
	endif()

	if (NOT EXISTS "${DIR_TEMP}/spine-c-2.1.27-main")
		file(ARCHIVE_EXTRACT INPUT "${SPINE_21_RUNTIME_BINARY_READER_FILENAME}" DESTINATION "${DIR_TEMP}")
	endif()

	file(COPY "${DIR_TEMP}/spine-c-2.1.27-main/spine-c/src" DESTINATION "${DIR_SPINE_C_21}")
	file(COPY "${DIR_TEMP}/spine-c-2.1.27-main/spine-c/include" DESTINATION "${DIR_SPINE_C_21}")
endif()

# Modification on spine-c 3.5 extension.c and extension.h
function(replace_names_in_file FILE_PATH OLD_NAME NEW_NAME)
	file(READ ${FILE_PATH} FILE)
	string(REPLACE ${OLD_NAME} ${NEW_NAME} MODIFIED_FILE "${FILE}")
	file(WRITE ${FILE_PATH} "${MODIFIED_FILE}")
endfunction()

function(MODIFY_SPINE_35)
	set(DIR_SPINE_C_35 ${CMAKE_CURRENT_LIST_DIR}/spine-c-3.5)
	set(NAMES_TO_BE_RENAMED "_malloc" "_calloc" "_realloc" "_free")
	set(FILES_TO_BE_MODIFIED "${DIR_SPINE_C_35}/src/spine/extension.c" "${DIR_SPINE_C_35}/include/spine/extension.h")

	foreach(FILENAME IN LISTS FILES_TO_BE_MODIFIED)
		foreach(NAME_TO_BE_RENAMED IN LISTS NAMES_TO_BE_RENAMED)
			string(REPLACE "_" "_sp" TEMP1 "${NAME_TO_BE_RENAMED}")
			string(SUBSTRING ${TEMP1} 3 1 TEMP2)
			string(TOUPPER ${TEMP2} TEMP3)
			string(SUBSTRING ${TEMP1} 4 -1 TEMP2)
			replace_names_in_file(${FILENAME} ${NAME_TO_BE_RENAMED} "_sp${TEMP3}${TEMP2}")
		endforeach()
	endforeach()

	# extension.h contains "char* _readFile" and "char* _spUtil_readFile".
	# The latter should not be overwritten.
	foreach(FILENAME IN LISTS FILES_TO_BE_MODIFIED)
		replace_names_in_file(${FILENAME} "char* _readFile" "char* _spReadFile")
	endforeach()
endfunction()

MODIFY_SPINE_35()

# Download Dear-ImGui docking
if (NOT EXISTS "${DIR_IMGUI}")
	set(IMGUI_SRC_URL "https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_VERSION}.zip")
	set(IMGUI_SRC_ARCHIVE_PATH "${DIR_TEMP}/${IMGUI_VERSION}")

	if (NOT EXISTS "${IMGUI_SRC_ARCHIVE_PATH}.zip")
		file(DOWNLOAD "${IMGUI_SRC_URL}" "${IMGUI_SRC_ARCHIVE_PATH}.zip" SHOW_PROGRESS)
	endif()

	if (NOT EXISTS "${IMGUI_SRC_ARCHIVE_PATH}")
		file(ARCHIVE_EXTRACT INPUT "${IMGUI_SRC_ARCHIVE_PATH}.zip" DESTINATION "${DIR_TEMP}")
	endif()

	file(COPY "${DIR_TEMP}/imgui-${IMGUI_VERSION}/" DESTINATION "${DIR_IMGUI}")
endif()

# Delete Spine runtimes for other languages because they are irrelevant for the project.
file(REMOVE_RECURSE "${DIR_TEMP}")

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>,$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>>")
endif()

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/d1trimfile:${CMAKE_SOURCE_DIR}\\>")
endif()

# Build Spine runtime as static library
add_subdirectory ("spine-c-2.1")
add_subdirectory ("spine-c-3.5")
add_subdirectory ("spine-c-3.6")
add_subdirectory ("spine-c-3.7")

add_subdirectory ("spine-cpp-3.8")
add_subdirectory ("spine-cpp-4.0")
add_subdirectory ("spine-cpp-4.1")
add_subdirectory ("spine-cpp-4.2")
