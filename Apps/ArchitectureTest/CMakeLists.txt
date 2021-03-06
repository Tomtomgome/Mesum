set(APP_NAME ArchitectureTest)

project(${APP_NAME} VERSION 1.0.0 DESCRIPTION "Small test for Cmake")

set(SOURCES main.cpp)
add_executable(${APP_NAME} ${SOURCES})
target_link_libraries(${APP_NAME} PUBLIC MesumCore)
set_target_properties(${APP_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

#add_custom_command(TARGET ${APP_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "${CMAKE_BINARY_DIR}/Engine/MesumCore/$<CONFIGURATION>/MesumCore.dll"
#        $<TARGET_FILE_DIR:${APP_NAME}>)
