set(APP_NAME CubeMover)

project(${APP_NAME} VERSION 1.0.0 DESCRIPTION "Small test application for moving a cube")

set(SOURCES CubeMoverApp.cpp)
add_executable(${APP_NAME} ${APP_DECLARE} ${SOURCES})
target_link_libraries(${APP_NAME} PUBLIC CrossPlatform Application)
set_target_properties(${APP_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(${APP_NAME} PRIVATE . ${ENGINE_SRC_DIR})