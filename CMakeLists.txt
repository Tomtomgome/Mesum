cmake_minimum_required(VERSION 3.20.2)

project(mesum-engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

set(ENGINE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Src)
set(EXTERNALS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Externals)

if (UNIX)
    add_compile_definitions(_M_UNIX)
    set(BM_APP_WINDOWED "")
    set(BM_UNIX "TRUE")
elseif(WIN32)
    add_compile_definitions(_M_WIN32)
    set(BM_APP_WINDOWED WIN32)
    set(BM_WIN32 "TRUE")
endif ()

#------------------------------------------------------------------------------
#User configuration
#------------------------------------------------------------------------------
#Build as dynamic
set(BM_DYNAMIC_LINK "FALSE")

#Are all the renderer available to the applications
set(BM_ALL_RENDERER "TRUE")
#Defines the default renderer (BM_DX12/BM_VULKAN) (mendatory)
set(BM_DEFAULT_RENDERER "BM_VULKAN")



#------------------------------------------------------------------------------
#Implementation
#------------------------------------------------------------------------------
#renderer defines
if(BM_DEFAULT_RENDERER STREQUAL "BM_DX12")
    if(BM_UNIX)
        message(FATAL_ERROR "There is no support for DX12 on unix platforms, can't be default renderer")
    endif()
    set(BM_DX12_DEFAULT_RENDERER "TRUE")
    add_compile_definitions(_M_DX12_DEFAULT_RENDERER)
elseif(BM_DEFAULT_RENDERER STREQUAL "BM_VULKAN")
    set(BM_VULKAN_DEFAULT_RENDERER "TRUE")
    add_compile_definitions(_M_VULKAN_DEFAULT_RENDERER)
endif()

if(BM_ALL_RENDERER)
    add_compile_definitions(_M_ALL_RENDERER)
    if(BM_UNIX)
        message(WARNING "There is no support for DX12 on unix platforms")
    else()
        list(APPEND BM_RENDERERS DX12Renderer)
        add_compile_definitions(_M_DX12_RENDERER)
        set(BM_DX12_RENDERER "TRUE")
    endif()
    list(APPEND BM_RENDERERS VulkanRenderer)
    add_compile_definitions(_M_VULKAN_RENDERER)
    set(BM_VULKAN_RENDERER "TRUE")
else()
    if(BM_DX12_DEFAULT_RENDERER)
        if(BM_UNIX)
            message(FATAL_ERROR "There is no support for DX12 on unix platforms")
        endif()
        list(APPEND BM_RENDERERS DX12Renderer)
        add_compile_definitions(_M_DX12_RENDERER)
    elseif(BM_VULKAN_DEFAULT_RENDERER)
        list(APPEND BM_RENDERERS VulkanRenderer)
        add_compile_definitions(_M_VULKAN_RENDERER)
    endif()
endif()

#Build defines
if(BM_DYNAMIC_LINK)
    add_compile_definitions(_M_DLL_DYNAMIC_LINK)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Engine)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Apps)