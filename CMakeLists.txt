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
elseif(WIN32)
add_compile_definitions(_M_WIN32)
set(BM_APP_WINDOWED WIN32)
endif ()


set(BM_DX12_RENDERER "TRUE")
set(BM_VULKAN_RENDERER "FALSE")

set(BM_DYNAMIC_LINK "FALSE")


#add_subdirectory(${CMAKE_SOURCE_DIR}/Src)

add_subdirectory(${CMAKE_SOURCE_DIR}/Engine)

add_subdirectory(${CMAKE_SOURCE_DIR}/Apps)