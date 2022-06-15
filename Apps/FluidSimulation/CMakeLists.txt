set(APP_NAME FluidSimulation)

project(${APP_NAME} VERSION 1.0.0 DESCRIPTION "Small test for Cmake and mesum graphics")

set(SOURCES main.cpp RenderTaskFluidSimulation.cpp RenderTaskFluidSimulation.hpp)
add_executable(${APP_NAME} ${BM_APP_WINDOWED} ${SOURCES})
target_link_libraries(${APP_NAME} PUBLIC MesumGraphics)
set_target_properties(${APP_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

if(BM_DYNAMIC_LINK)

add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/Engine/MesumCore/$<CONFIGURATION>/MesumCore.dll"
        $<TARGET_FILE_DIR:${APP_NAME}>)

add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_BINARY_DIR}/Engine/MesumGraphics/$<CONFIGURATION>/MesumGraphics.dll"
        $<TARGET_FILE_DIR:${APP_NAME}>)

endif()