set(MODULE_NAME DX12Renderer)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "DX12Renderer module")

set(SOURCES DX12Renderer.cpp)
set(HEADERS DX12Renderer.hpp
            d3dx12.h)

add_library(${MODULE_NAME} ${SOURCES} ${HEADERS})

set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${MODULE_NAME} PROPERTIES PUBLIC_HEADER DX12Renderer.hpp)
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "Render")

target_link_libraries(${MODULE_NAME} PUBLIC Logger Kernel d3d12.lib dxgi.lib INTERFACE Math)

target_include_directories(${MODULE_NAME} PUBLIC  . ..)