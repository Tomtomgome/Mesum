cmake_minimum_required(VERSION 3.10)

set(MODULE_NAME Kernel)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "Types and asserts")

set(SOURCES )
set(HEADERS Types.hpp
            Asserts.hpp)

add_library(${MODULE_NAME} INTERFACE)

target_link_libraries(${APP_NAME} Logger)

#No version on interface set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
#No public headers on interfaces set_target_properties(${MODULE_NAME} PROPERTIES PUBLIC_HEADER HEADERS)

target_include_directories(${MODULE_NAME} INTERFACE .)
