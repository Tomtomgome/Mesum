set(MODULE_NAME DearImguiBase)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "Base Imgui lib")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_PROTOTYPES")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_PROTOTYPES")

set(SOURCES imgui_demo.cpp
            imgui_draw.cpp
            imgui_tables.cpp
            imgui_widgets.cpp
            imgui.cpp)

set(HEADERS imconfig.h
            imgui_internal.h
            imgui.h
            imstb_rectpack.h
            imstb_textedit.h
            imstb_truetype.h)

add_library(${MODULE_NAME} OBJECT ${SOURCES} ${HEADERS})

set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "MesumGraphics")
set_target_properties(${MODULE_NAME} PROPERTIES UNITY_BUILD ON)

target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${MODULE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(${MODULE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../..)