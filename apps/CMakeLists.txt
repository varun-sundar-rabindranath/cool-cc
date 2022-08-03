cmake_minimum_required(VERSION 3.16.3)

project(apps)

# C++
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Directories
# For the correct environment variables to be set it is required this cmake is
# executed from within the cool-cc init shell
# TODO - This should really be a find_package kind of thing
set(COMPILER_INCLUDE_DIR ${COMPILER_INSTALL_DIR}/include)

# Includes
include_directories(${COMPILER_INCLUDE_DIR})

# Runtime output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set(CMAKE_INSTALL_RPATH $ENV{COMPILER_INSTALL_DIR}/lib:$ENV{PL_INSTALL_DIR}/lib)

# Required external libraries
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

## Add compiler_app - arith
set(ARITH_EXECUTABLE_NAME compiler_app_arith)

add_executable(${ARITH_EXECUTABLE_NAME}
               compiler_app.cpp)

target_link_libraries(${ARITH_EXECUTABLE_NAME}
		      lexer
                      recursive_descent_parser
		      fmt::fmt)

install(TARGETS ${ARITH_EXECUTABLE_NAME} DESTINATION bin)

