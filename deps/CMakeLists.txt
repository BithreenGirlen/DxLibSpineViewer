cmake_minimum_required (VERSION 3.25)
project(DxLibSpineViewer-deps)

set(IMGUI_VERSION "1.92.6-docking")
set(IMGUI_DIR "${CMAKE_CURRENT_LIST_DIR}/imgui-docking")

set(DXLIB_DIR ${CMAKE_CURRENT_LIST_DIR}/dxlib)

set(TEMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/temp)

function(download_and_extract HOST_URL ARCHIVE_NAME)
	if (NOT EXISTS "${TEMP_DIR}/${ARCHIVE_NAME}.zip")
		file(DOWNLOAD "${HOST_URL}" "${TEMP_DIR}/${ARCHIVE_NAME}.zip" SHOW_PROGRESS)
	endif()

	if (NOT EXISTS "${TEMP_DIR}/${ARCHIVE_NAME}")
		file(ARCHIVE_EXTRACT INPUT "${TEMP_DIR}/${ARCHIVE_NAME}.zip" DESTINATION "${TEMP_DIR}")
	endif()
endfunction()

# Download DxLib.
# --------------------
# The version to be used is the latest, so download from "../temp/DxLibVCTest.zip".
# The static libraries in need are those for Visual Studio x64 MDd or MD.

if (NOT EXISTS "${DXLIB_DIR}")
	set (DXLIB_VC_URL "https://dxlib.xsrv.jp/temp/DxLibVCTest.zip")
	set(DXLIB_VC_FOLDER_NAME "DxLibVCTest")
	download_and_extract("${DXLIB_VC_URL}" "${DXLIB_VC_FOLDER_NAME}")

	file(GLOB DXLIB_LIBS "${TEMP_DIR}/${DXLIB_VC_FOLDER_NAME}/*.lib")
	foreach(FILE IN LISTS DXLIB_LIBS)
		if(FILE MATCHES "_vs2015_x64_MDd.lib" OR FILE MATCHES "_vs2015_x64_MD.lib")
			file(COPY ${FILE} DESTINATION "${DXLIB_DIR}")
		endif()
	endforeach()

	file(GLOB INCLUDE_FILES "${TEMP_DIR}/DxLibVCTest/*.h")
	foreach(FILE IN LISTS INCLUDE_FILES)
		file(COPY ${FILE} DESTINATION "${DXLIB_DIR}")
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
	set (SPINE_FOLDER_NAME "spine-runtimes-${BRANCH_NAME}")
	download_and_extract("${SPINE_RUNTIME_URL}" "${SPINE_FOLDER_NAME}")

	if (FOLDER_PATH MATCHES "spine-c-")
		file(COPY "${TEMP_DIR}/${SPINE_FOLDER_NAME}/spine-c/spine-c/src" DESTINATION "${FOLDER_PATH}")
		file(COPY "${TEMP_DIR}/${SPINE_FOLDER_NAME}/spine-c/spine-c/include" DESTINATION "${FOLDER_PATH}")
	else()
		file(COPY "${TEMP_DIR}/${SPINE_FOLDER_NAME}/spine-cpp/spine-cpp/src" DESTINATION "${FOLDER_PATH}")
		file(COPY "${TEMP_DIR}/${SPINE_FOLDER_NAME}/spine-cpp/spine-cpp/include" DESTINATION "${FOLDER_PATH}")
	endif()
endfunction()

foreach(VERSION IN LISTS SPINE_VERSIONS)
	string(REPLACE ":" ";" VERSIONS ${VERSION})
	list(GET VERSIONS 0 BRANCH_NAME)
	list(GET VERSIONS 1 FOLDER_NAME)

	set(SPINE_DIR "${CMAKE_CURRENT_LIST_DIR}/${FOLDER_NAME}")

	if (NOT EXISTS "${SPINE_DIR}/src")
		donwload_spine_runtime(${BRANCH_NAME} ${SPINE_DIR})
	endif()
endforeach()

# Exceptional procedure for spine 2.1 runtime
set(SPINE_C_21_DIR ${CMAKE_CURRENT_LIST_DIR}/spine-c-2.1)
if (NOT EXISTS "${SPINE_C_21_DIR}/src")
	set (SPINE_21_RUNTIME_URL "https://github.com/EsotericSoftware/spine-runtimes/archive/refs/tags/2.1.25.zip")
	set (SPINE_21_FOLDER_NAME "spine-runtimes-2.1.25")
	download_and_extract("${SPINE_21_RUNTIME_URL}" "${SPINE_21_FOLDER_NAME}")
	
	# One depth shorter than later versions.
	file(COPY "${TEMP_DIR}/${SPINE_21_FOLDER_NAME}/spine-c/src" DESTINATION "${SPINE_C_21_DIR}")
	file(COPY "${TEMP_DIR}/${SPINE_21_FOLDER_NAME}/spine-c/include" DESTINATION "${SPINE_C_21_DIR}")

	# Add skeleton binary reader and overwrite some of the files for spine 2.1.
	set (SPINE_21_RUNTIME_BINARY_READER_URL "https://github.com/BithreenGirlen/spine-c-2.1.27/archive/refs/heads/main.zip")
	set (SPINE_21_RUNTIME_BINARY_READER_FOLDER_NAME "spine-c-2.1.27")
	download_and_extract("${SPINE_21_RUNTIME_BINARY_READER_URL}" "${SPINE_21_RUNTIME_BINARY_READER_FOLDER_NAME}")

	file(COPY "${TEMP_DIR}/${SPINE_21_RUNTIME_BINARY_READER_FOLDER_NAME}-main/spine-c/src" DESTINATION "${SPINE_C_21_DIR}")
	file(COPY "${TEMP_DIR}/${SPINE_21_RUNTIME_BINARY_READER_FOLDER_NAME}-main/spine-c/include" DESTINATION "${SPINE_C_21_DIR}")
endif()

# Modification on spine-c 3.5 extension.c and extension.h
function(replace_names_in_file FILE_PATH OLD_NAME NEW_NAME)
	file(READ ${FILE_PATH} FILE)
	string(REPLACE ${OLD_NAME} ${NEW_NAME} MODIFIED_FILE "${FILE}")
	file(WRITE ${FILE_PATH} "${MODIFIED_FILE}")
endfunction()

function(MODIFY_SPINE_35)
	set(SPINE_C_35_DIR ${CMAKE_CURRENT_LIST_DIR}/spine-c-3.5)
	set(NAMES_TO_BE_RENAMED "_malloc" "_calloc" "_realloc" "_free")
	set(FILES_TO_BE_MODIFIED "${SPINE_C_35_DIR}/src/spine/extension.c" "${SPINE_C_35_DIR}/include/spine/extension.h")

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
if (NOT EXISTS "${IMGUI_DIR}")
	set (IMGUI_FOLDER_NAME "imgui-${IMGUI_VERSION}")
	download_and_extract("https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_VERSION}.zip" "${IMGUI_FOLDER_NAME}")
	# Exclude VC project files, documents, and examples
	file(COPY "${TEMP_DIR}/${IMGUI_FOLDER_NAME}/"
	DESTINATION "${IMGUI_DIR}"
	FILES_MATCHING 
		PATTERN "*.cpp"
		PATTERN "*.h"
		PATTERN ".github" EXCLUDE
		PATTERN "docs" EXCLUDE
		PATTERN "examples" EXCLUDE
		PATTERN "misc" EXCLUDE
	)
endif()

# Delete Spine runtimes for other languages because they are irrelevant for the project.
file(REMOVE_RECURSE "${TEMP_DIR}")
