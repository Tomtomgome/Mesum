cmake_minimum_required(VERSION 3.10)

set(MODULE_NAME Logger)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "Thread safe logging library")

set(SOURCES Log.cpp
            LogPolicies.cpp)
set(HEADERS Logger.hpp
			LogConfig.hpp
            Log.hpp
            LogPolicies.hpp)

add_library(${MODULE_NAME} ${SOURCES} ${HEADERS})

set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${MODULE_NAME} PROPERTIES PUBLIC_HEADER Logger.hpp)
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "Kernel")

target_include_directories(${MODULE_NAME} PUBLIC .)
