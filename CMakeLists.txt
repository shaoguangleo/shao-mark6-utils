cmake_minimum_required(VERSION 3.2)
project(SHAOMark6Utils)

# Version number

set(SHAOMark6Utils_VERSION_MAJOR 1)
set(SHAOMark6Utils_VERSION_MINOR 0)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Threads)

add_subdirectory(src )
add_subdirectory(include)
add_subdirectory(scripts)

#target_include_directories(SHAOMark6Utils PUBLIC "${PROJECT_BINARY_DIR}")

include_directories(include)