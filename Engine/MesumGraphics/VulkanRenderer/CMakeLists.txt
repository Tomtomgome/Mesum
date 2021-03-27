set(MODULE_NAME VulkanRenderer)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "Win32 specific windows")

set(SOURCES VulkanContext.cpp
            VulkanRendererCommon.cpp
            imgui_impl_vulkan.cpp)

set(HEADERS VulkanContext.hpp
            VulkanRendererCommon.hpp
            imgui_impl_vulkan.h
            Includes.hpp
            ../MesumGraphics/Common.hpp)

add_library(${MODULE_NAME} OBJECT ${SOURCES} ${HEADERS} $<TARGET_OBJECTS:DearImguiBase>)

find_package(Vulkan REQUIRED)

target_link_libraries(${MODULE_NAME} MesumCore DearImguiBase Vulkan::Vulkan)

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