cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_CXX_STANDARD 11)
project(haxy)

SET (CMAKE_C_COMPILER             "/usr/bin/clang")
SET (CMAKE_CXX_COMPILER           "/usr/bin/clang++")

# Find required packages
find_package(PkgConfig)

#pkg_check_modules(rd REQUIRED Readline)

# Main executable
#include_directories(include)
file(GLOB SOURCES "src/*.cpp" "src/mpc/mpc.c" "src/sherwood_map/sherwood_map.cpp")
add_executable(haxy ${SOURCES})

# Libraries
target_compile_options(haxy PRIVATE -g)
target_link_libraries(haxy readline m)
target_link_libraries(haxy)

# Installation
install(TARGETS haxy DESTINATION bin)
install(DIRECTORY src/lexer DESTINATION share/haxy)
