cmake_minimum_required (VERSION 3.25)

set(SPINE_VERSION "3.5")
set(SPINE_RUNTIME_LANGUAGE "spine-c")
set(SPINE_LIBRARY_NAME "${SPINE_RUNTIME_LANGUAGE}-${SPINE_VERSION}")

file(GLOB INCLUDES "include/**/*.h")
file(GLOB SOURCES "src/**/*.c")

add_library("${SPINE_LIBRARY_NAME}" STATIC ${SOURCES} ${INCLUDES})
target_include_directories("${SPINE_LIBRARY_NAME}" PUBLIC include)

target_compile_features("${SPINE_LIBRARY_NAME}" PUBLIC cxx_std_14)
set_target_properties("${SPINE_LIBRARY_NAME}" PROPERTIES DEBUG_POSTFIX "-d")
set_target_properties("${SPINE_LIBRARY_NAME}" PROPERTIES ARCHIVE_OUTPUT_NAME "${SPINE_RUNTIME_LANGUAGE}")

install(
	TARGETS "${SPINE_LIBRARY_NAME}"
	ARCHIVE DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/"
)
