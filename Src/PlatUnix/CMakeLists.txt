cmake_minimum_required(VERSION 3.10)

set(MODULE_NAME PlatUnix)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "Unix specific apis")

set(SOURCES UnixApp.cpp)
set(HEADERS UnixApp.hpp)

add_library(${MODULE_NAME} ${SOURCES} ${HEADERS})

set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${MODULE_NAME} PROPERTIES PUBLIC_HEADER UnixApp.hpp)

target_link_libraries(${MODULE_NAME} PUBLIC Logger X11 Input INTERFACE Math)

target_include_directories(${MODULE_NAME} PUBLIC  . ..)