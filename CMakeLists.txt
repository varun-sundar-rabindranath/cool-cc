cmake_minimum_required(VERSION 3.16.3)

project(cool-cc)

# Directories
set(PROJECT_INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/include)
set(CMAKE_BUILD_DIRECTORY ${PROJECT_SOURCE_DIR}/out/build)
set(CMAKE_BINARY_DIRECTORY ${PROJECT_SOURCE_DIR}/out/build/out) # Path to store build artifacts

# C++
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Includes
include_directories(${PROJECT_INCLUDE_DIRECTORY})

# Runtime output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

# Required external libraries
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

add_subdirectory(src)

