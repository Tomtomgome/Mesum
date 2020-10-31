cmake_minimum_required(VERSION 3.10)

set(MODULE_NAME Input)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "Independant input module")

set(SOURCES 	Input.cpp)
set(HEADERS 	Input.hpp
				Keys.hpp
		InputCommon.hpp)

add_library(${MODULE_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${MODULE_NAME} PUBLIC Kernel INTERFACE Math)

set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${MODULE_NAME} PROPERTIES PUBLIC_HEADER Input.hpp)
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "Kernel")

target_include_directories(${MODULE_NAME} PUBLIC . ..)
