add_subdirectory(Kernel)

set(MODULE_NAME MesumCore)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "MesumEngine core functionnalities")

if(BM_DYNAMIC_LINK)
set(BM_LINK_MODE DYNAMIC)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
else()
set(BM_LINK_MODE STATIC)
endif()


add_library(${MODULE_NAME} ${BM_LINK_MODE} includes.hpp $<TARGET_OBJECTS:Kernel2>)

target_link_libraries(${MODULE_NAME} Kernel2)

set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties(${MODULE_NAME} PROPERTIES PUBLIC_HEADER HEADERS)
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "MesumCore")

target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${MODULE_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)


