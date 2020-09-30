
add_subdirectory(Logger)
add_subdirectory(Kernel)
add_subdirectory(Simd)
add_subdirectory(Math)

set(APP_NAME MesumMain)

project(${APP_NAME} VERSION 1.0.0 DESCRIPTION "Main application used for practicing functionalities")

set(SOURCES main.cpp)

add_executable(${APP_NAME} ${SOURCES})

target_link_libraries(${APP_NAME} PUBLIC Logger Simd INTERFACE Math)

set_target_properties(${APP_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

target_include_directories(${APP_NAME} PRIVATE .)

