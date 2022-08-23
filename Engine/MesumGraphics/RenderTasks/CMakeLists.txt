set(MODULE_NAME RenderTasks)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "MesumEngine Basic graphic tasks")

set(SOURCES RenderTaskDearImGui.cpp
        RenderTasksBasicSwapchain.cpp
        RenderTask2DRender.cpp
        RenderTask3dRender.cpp)

set(HEADERS RenderTaskDearImGui.hpp
        RenderTasksBasicSwapchain.hpp
        RenderTask2DRender.hpp
        RenderTask3dRender.hpp
        Mesh.cpp
        Mesh.hpp
        BasicVertex.cpp
        BasicVertex.hpp)

foreach(RENDER_MODULE_NAME ${RENDERER_MODULE_NAMES})
    list(APPEND TARGET_OBJECTS $<TARGET_OBJECTS:${RENDER_MODULE_NAME}>)
endforeach()

add_library(${MODULE_NAME} OBJECT ${SOURCES} ${HEADERS} ${TARGET_OBJECTS} $<TARGET_OBJECTS:DearImguiBase>)

target_link_libraries(${MODULE_NAME} MesumCore ${RENDERER_MODULE_NAMES} DearImguiBase)

set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties(${MODULE_NAME} PROPERTIES PUBLIC_HEADER HEADERS)
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "MesumGraphics")
set_target_properties(${MODULE_NAME} PROPERTIES UNITY_BUILD ON)

target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${MODULE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(${MODULE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../..)

if(BM_DYNAMIC_LINK)
target_compile_definitions(${MODULE_NAME} PRIVATE _M_GRAPHICS_EXPORT)
endif()
