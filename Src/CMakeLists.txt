cmake_minimum_required(VERSION 3.12)

set(ENGINE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if (UNIX)
add_compile_definitions(M_UNIX)
set(APP_DECLARE "")
elseif(WIN32)
add_compile_definitions(M_WINDOWS)
set(APP_DECLARE WIN32)
endif ()

add_subdirectory(Logger)
add_subdirectory(Kernel)
add_subdirectory(Input)
add_subdirectory(Simd)
add_subdirectory(Math)
add_subdirectory(Application)

add_subdirectory(PlatUnix)
add_subdirectory(PlatWindows)

add_subdirectory(CrossPlatform)

add_subdirectory(Apps)

set(APP_NAME MesumMain)
project(${APP_NAME} VERSION 1.0.0 DESCRIPTION "Main application used for practicing functionalities")
set(SOURCES main.cpp)
add_executable(${APP_NAME} ${SOURCES})
target_link_libraries(${APP_NAME} PUBLIC Logger Simd INTERFACE Math)
set_target_properties(${APP_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(${APP_NAME} PRIVATE .)

