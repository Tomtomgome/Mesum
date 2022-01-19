set(APP_NAME GameWindow)

project(${APP_NAME} VERSION 1.0.0 DESCRIPTION "Game about transparent windows")

if(BM_DX12_RENDERER)
add_compile_definitions(_M_DX12_RENDERER)
elseif(BM_VULKAN_RENDERER)
add_compile_definitions(_M_VULKAN_RENDERER)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES main.cpp)
add_executable(${APP_NAME} ${BM_APP_WINDOWED} ${SOURCES})
target_link_libraries(${APP_NAME} PUBLIC MesumGraphics)
target_include_directories(${APP_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.)

set_target_properties(${APP_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

if(BM_DYNAMIC_LINK)

    add_custom_command(TARGET ${APP_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/Engine/MesumCore/MesumCore.dll"
            $<TARGET_FILE_DIR:${APP_NAME}>)

    add_custom_command(TARGET ${APP_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/Engine/MesumGraphics/MesumGraphics.dll"
            $<TARGET_FILE_DIR:${APP_NAME}>)

endif()