set(APP_NAME MathTest)

project(${APP_NAME} VERSION 1.0.0 DESCRIPTION "Small test application for math lib")

set(SOURCES main.cpp)
add_executable(${APP_NAME} ${APP_DECLARE} ${SOURCES})
target_link_libraries(${APP_NAME} PUBLIC Logger Kernel Simd INTERFACE Math)
set_target_properties(${APP_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(${APP_NAME} PRIVATE . ${ENGINE_SRC_DIR})