set(MODULE_NAME DX12Renderer)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "Win32 specific windows")

set(SOURCES DX12Renderer.cpp)

set(HEADERS DX12Renderer.hpp
            d3dx12.h
            ../MesumGraphics/Common.hpp)

add_library(${MODULE_NAME} OBJECT ${SOURCES} ${HEADERS})

target_link_libraries(${MODULE_NAME} MesumCore d3d12.lib dxgi.lib)

#target_precompile_headers(${MODULE_NAME} PRIVATE Kernel_pch.hpp)
set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "MesumGraphics")
set_target_properties(${MODULE_NAME} PROPERTIES UNITY_BUILD ON)

target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${MODULE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(${MODULE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../..)

if(BM_DYNAMIC_LINK)
target_compile_definitions(${MODULE_NAME} PRIVATE _M_DLL_DYNAMIC_LINK)
target_compile_definitions(${MODULE_NAME} PRIVATE _M_EXPORT)
endif()