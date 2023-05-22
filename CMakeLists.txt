cmake_minimum_required(VERSION 3.20.2)

project(mesum-engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(ENGINE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Src)
set(EXTERNALS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Externals)

if (NOT BM_CUSTOM_BUILD)
    set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

    #------------------------------------------------------------------------------
    #User configuration
    #------------------------------------------------------------------------------
    #Build as dynamic
    set(BM_DYNAMIC_LINK "FALSE")
    #Are all the renderer available to the applications
    set(BM_ALL_RENDERER "TRUE")
    #Defines the default renderer (BM_DX12/BM_VULKAN) (mendatory)
    set(BM_DEFAULT_RENDERER "BM_VULKAN")

    include(CMakeSetup.cmake)
    setup_config()
endif ()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Engine)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Apps)