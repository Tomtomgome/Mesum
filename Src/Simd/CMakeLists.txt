set(MODULE_NAME Simd)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "Simd encapsulation library")

set(SOURCES SimdTypes.cpp)
set(HEADERS SimdTypes.hpp)

add_library(${MODULE_NAME} ${SOURCES} ${HEADERS})

set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${MODULE_NAME} PROPERTIES PUBLIC_HEADER SimdType.hpp)
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "Kernel")

target_include_directories(${MODULE_NAME} PUBLIC .)
