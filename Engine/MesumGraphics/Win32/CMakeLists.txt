set(MODULE_NAME PlatWin32)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "Win32 specific windows")

set(SOURCES Win32Context.cpp
            WindowedAppImpl.cpp
            WindowsImpl.cpp
            imgui_impl_win32.cpp
            )
set(HEADERS Win32Context.hpp
            WindowedAppImpl.hpp
            WindowsImpl.hpp
            Includes.hpp
            imgui_impl_win32.h
            ../MesumGraphics/Common.hpp)

add_library(${MODULE_NAME} OBJECT ${SOURCES} ${HEADERS} $<TARGET_OBJECTS:${RENDERER_MODULE_NAME}> $<TARGET_OBJECTS:DearImguiBase>)

target_link_libraries(${MODULE_NAME} MesumCore ${RENDERER_MODULE_NAME} DearImguiBase)

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