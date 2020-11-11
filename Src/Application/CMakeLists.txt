set(MODULE_NAME Application)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "Application framework")

set(SOURCES 	Application.cpp)
set(HEADERS 	Application.hpp
                Main.hpp)

add_library(${MODULE_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${MODULE_NAME} PUBLIC Kernel)

set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties(${MODULE_NAME} PROPERTIES PUBLIC_HEADER ${HEADERS})
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "Application")

target_include_directories(${MODULE_NAME} PUBLIC . ..)