cmake_minimum_required(VERSION 3.27)

# Compile with C++14
set (CMAKE_CXX_STANDARD 14)

# Compile as release
set (CMAKE_BUILD_TYPE RELEASE)
set (CMAKE_CXX_FLAGS_RELEASE "-O3")

# Set the project name to INF273
project(INF273)

# Set the header file directory
include_directories(src/headers)

# Add all files to compile with
file(GLOB SOURCES
    src/*.h
    src/*.cpp
)

# Compile to 'run.exe' executable
add_executable(run.exe ${SOURCES})
