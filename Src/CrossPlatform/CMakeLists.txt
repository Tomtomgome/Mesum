set(MODULE_NAME CrossPlatform)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "cross platform framework")

set(SOURCES 	CrossPlatform.cpp)
set(HEADERS 	CrossPlatform.hpp)

add_library(${MODULE_NAME} ${SOURCES} ${HEADERS})

if(UNIX)
target_link_libraries(${MODULE_NAME} PlatUnix)
elseif(WIN32)
target_link_libraries(${MODULE_NAME} PlatWindows)
endif()

#target_link_libraries(${MODULE_NAME} INTERFACE Kernel)

set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties(${MODULE_NAME} PROPERTIES PUBLIC_HEADER ${HEADERS})
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "CrossPlatform")

target_include_directories(${MODULE_NAME} PUBLIC . ..)