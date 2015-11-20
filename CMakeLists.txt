cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_CXX_STANDARD 11)
project(haxy)


# Find required packages
find_package(PkgConfig)

#pkg_check_modules(rd REQUIRED Readline)

# Main executable
#include_directories(include)
file(GLOB SOURCES "src/*.c*")
add_executable(haxy ${SOURCES})

# Libraries
target_link_libraries(haxy readline m)

# Installation
install(TARGETS haxy DESTINATION bin)
