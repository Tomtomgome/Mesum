set(MODULE_NAME Kernel)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "Types and asserts")

set(SOURCES Kernel.cpp)
set(HEADERS Types.hpp
            Asserts.hpp
            Callbacks.hpp)

add_library(${MODULE_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${MODULE_NAME} Logger)

set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties(${MODULE_NAME} PROPERTIES PUBLIC_HEADER HEADERS)
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "Kernel")

target_include_directories(${MODULE_NAME} PUBLIC . ..)
