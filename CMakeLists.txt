cmake_minimum_required(VERSION 3.9.1)

set (CMAKE_CXX_STANDARD 11)

project(INF273)

file(GLOB SOURCES
    src/*.h
    src/*.cpp
)

add_executable(run.exe ${SOURCES})
