cmake_minimum_required(VERSION 3.20)
project(DxLibSpinePlayer-dll)

set(CMAKE_CXX_STANDARD 14)

# Download dependencies
add_subdirectory(deps)

# Build Spine runtime for DxLib as shared library
add_subdirectory(runtime)

# Build Spine viewer
add_subdirectory(viewer)
