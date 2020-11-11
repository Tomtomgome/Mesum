set(MODULE_NAME Math)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "Simple math library")

set(SOURCES )
set(HEADERS MathTypes.hpp
            Vec.hpp
            Vec.inl)

add_library(${MODULE_NAME} INTERFACE)# ${SOURCES} ${HEADERS})

target_link_libraries(${MODULE_NAME} INTERFACE Simd)

#set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
#set_target_properties(${MODULE_NAME} PROPERTIES PUBLIC_HEADER HEADERS)

target_include_directories(${MODULE_NAME} INTERFACE  .)
