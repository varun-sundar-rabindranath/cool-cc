cmake_minimum_required(VERSION 3.16.3)

project(pl)

# Directories
# For the correct environment variables to be set it is required this cmake is
# executed from within the cool-cc init shell
# TODO - This should really be a find_package kind of thing
set(COMPILER_INSTALL_DIR $ENV{COMPILER_INSTALL_DIR})
set(COMPILER_INCLUDE_DIR ${COMPILER_INSTALL_DIR}/include)
set(COMPILER_LIB_DIR ${COMPILER_INSTALL_DIR}/lib)

# Includes
include_directories(${COMPILER_INCLUDE_DIR})

# Runtime output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
#set(CMAKE_INSTALL_RPATH $ENV{COOLCC_INSTALL_DIR}/lib)

# C++
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(arith)
