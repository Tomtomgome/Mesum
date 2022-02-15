add_subdirectory(DearImgui)

foreach(Renderer ${BM_RENDERERS})
    add_subdirectory(${Renderer})
    list(APPEND RENDERER_MODULE_NAMES ${Renderer})
endforeach()

message(STATUS "module names are ${RENDERER_MODULE_NAMES}")

add_subdirectory(Unix)
add_subdirectory(Win32)

#add_subdirectory(Resources)
add_subdirectory(RenderTasks)

set(MODULE_NAME MesumGraphics)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "MesumEngine core functionnalities")

set(SOURCES WindowedApp.cpp
            )
set(HEADERS Windows.hpp
            WindowedApp.hpp
            CrossPlatform.hpp
            Renderer.hpp
            CrossRenderer.hpp
            RenderTask.hpp
            ApiAbstraction.hpp
            MesumGraphics/Common.hpp)

if(BM_DYNAMIC_LINK)
set(BM_LINK_MODE SHARED)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
else()
set(BM_LINK_MODE STATIC)
endif()

if(WIN32)
set(PLATFORME_MODULE_NAME PlatWin32)
elseif(UNIX)
set(PLATFORME_MODULE_NAME PlatUnix)
endif()

add_library(${MODULE_NAME} ${BM_LINK_MODE} ${SOURCES} ${HEADERS} $<TARGET_OBJECTS:${PLATFORME_MODULE_NAME}> $<TARGET_OBJECTS:RenderTasks>)

target_link_libraries(${MODULE_NAME} MesumCore ${PLATFORME_MODULE_NAME} ${RENDERER_MODULE_NAMES} RenderTasks DearImguiBase )

set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties(${MODULE_NAME} PROPERTIES PUBLIC_HEADER HEADERS)
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "MesumGraphics")
set_target_properties(${MODULE_NAME} PROPERTIES UNITY_BUILD ON)

target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${MODULE_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(BM_DYNAMIC_LINK)
target_compile_definitions(${MODULE_NAME} PRIVATE _M_GRAPHICS_EXPORT)
endif()
