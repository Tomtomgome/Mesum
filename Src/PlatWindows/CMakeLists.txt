set(MODULE_NAME PlatWindows)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "Windows specific apis")

set(SOURCES WindowsApp.cpp)
set(HEADERS WindowsApp.hpp)

add_library(${MODULE_NAME} ${SOURCES} ${HEADERS})

set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${MODULE_NAME} PROPERTIES PUBLIC_HEADER WindowsApp.hpp)
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "Platform")

target_link_libraries(${MODULE_NAME} PUBLIC Logger Input Application INTERFACE Math)

target_include_directories(${MODULE_NAME} PUBLIC  . ..)