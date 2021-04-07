set(MODULE_NAME Kernel)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "Types, logs, asserts")

set(SOURCES Kernel.cpp
            Log.cpp
            LogPolicies.cpp
            Asserts.cpp
            Mains.cpp
            Application.cpp
            Input.cpp
            )
set(HEADERS Types.hpp
            Asserts.hpp
            Callbacks.hpp
            Kernel.hpp
            Logger.hpp
            Log.hpp
            LogPolicies.hpp
            Vec.hpp
            Vec.inl
            MathTypes.hpp
            Mains.hpp
            Application.hpp
            Kernel_pch.hpp
            Keys.hpp
            Input.hpp
            InputCommon.hpp
            ../MesumCore/Common.hpp)

add_library(${MODULE_NAME} OBJECT ${SOURCES} ${HEADERS})

#target_precompile_headers(${MODULE_NAME} PRIVATE Kernel_pch.hpp)
set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "MesumCore")
set_target_properties(${MODULE_NAME} PROPERTIES UNITY_BUILD ON)

target_include_directories(${MODULE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${MODULE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(BM_DYNAMIC_LINK)
target_compile_definitions(${MODULE_NAME} PRIVATE _M_CORE_EXPORT)
endif()