cmake_minimum_required(VERSION 3.27)

set (CMAKE_CXX_STANDARD 14)

project(INF273)

file(GLOB SOURCES
    src/*.h
    src/*.cpp
)

add_executable(run.exe ${SOURCES})
