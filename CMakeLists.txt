cmake_minimum_required(VERSION 3.10)

project(mesum-engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

add_subdirectory(${CMAKE_SOURCE_DIR}/Src)