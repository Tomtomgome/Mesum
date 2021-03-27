add_subdirectory(DearImgui)
add_subdirectory(VulkanRenderer)
add_subdirectory(DX12Renderer)

if(BM_DX12_RENDERER)
Set(RENDERER_MODULE_NAME DX12Renderer)
add_compile_definitions(_M_DX12_RENDERER)
elseif(BM_VULKAN_RENDERER)
Set(RENDERER_MODULE_NAME VulkanRenderer)
add_compile_definitions(_M_VULKAN_RENDERER)
endif()

add_subdirectory(Win32)

set(MODULE_NAME MesumGraphics)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "MesumEngine core functionnalities")

set(SOURCES WindowedApp.cpp
            )
set(HEADERS Windows.hpp
            WindowedApp.hpp
            CrossPlatform.hpp
            Renderer.hpp
            CrossRenderer.hpp
            MesumGraphics/Common.hpp)

if(BM_DYNAMIC_LINK)
set(BM_LINK_MODE DYNAMIC)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
else()
set(BM_LINK_MODE STATIC)
endif()

if(WIN32)
set(PLATFORME_MODULE_NAME PlatWin32)
elseif(UNIX)
set(PLATFORME_MODULE_NAME PlatUnix)
endif()

add_library(${MODULE_NAME} ${BM_LINK_MODE} ${SOURCES} ${HEADERS} $<TARGET_OBJECTS:${PLATFORME_MODULE_NAME}>)

target_link_libraries(${MODULE_NAME} MesumCore ${PLATFORME_MODULE_NAME} ${RENDERER_MODULE_NAME} DearImguiBase)

set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties(${MODULE_NAME} PROPERTIES PUBLIC_HEADER HEADERS)
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "MesumGraphics")
set_target_properties(${MODULE_NAME} PROPERTIES UNITY_BUILD ON)

target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${MODULE_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)


