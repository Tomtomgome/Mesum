cmake_minimum_required(VERSION 3.16)

project(mesum-engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

set(ENGINE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Src)

if (UNIX)
    add_compile_definitions(_M_UNIX)
    set(BM_APP_WINDOWED "")
    set(BM_UNIX "TRUE")
elseif(WIN32)
    add_compile_definitions(_M_WIN32)
    set(BM_APP_WINDOWED WIN32)
    set(BM_WIN32 "TRUE")
endif ()

#Build as dynamic
set(BM_DYNAMIC_LINK "FALSE")

#Are all the renderer available to the applications
set(BM_ALL_RENDERER "TRUE")
#Defines the default renderer (BM_DX12/BM_VULKAN) (mendatory)
set(BM_DEFAULT_RENDERER "BM_DX12")

if(BM_DEFAULT_RENDERER STREQUAL "BM_DX12")
    set(BM_DX12_RENDERER "TRUE")
elseif(BM_DEFAULT_RENDERER STREQUAL "BM_VULKAN")
    set(BM_VULKAN_RENDERER "TRUE")
endif()

if(BM_DYNAMIC_LINK)
    add_compile_definitions(_M_DLL_DYNAMIC_LINK)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/Engine)

add_subdirectory(${CMAKE_SOURCE_DIR}/Apps)