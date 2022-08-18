set(MODULE_NAME Kernel)

project(${MODULE_NAME} VERSION 1.0.0 DESCRIPTION "Types, logs, asserts")

set(SOURCES Kernel.cpp
		Log.cpp
		LogPolicies.cpp
		Asserts.cpp
		Mains.cpp
		Math.cpp
		Application.cpp
		Input.cpp
		File.cpp
		Image.cpp
		Profile.cpp
		Spherical.cpp
		StateInputManager.cpp
		MatHelpers.cpp
		Quaternion.cpp
		Serializable.cpp
		Memory.cpp
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
		Math.hpp
		Mains.hpp
		Application.hpp
		Kernel_pch.hpp
		Keys.hpp
		Input.hpp
		File.hpp
		Image.hpp
		Profile.hpp
		InputCommon.hpp
		../Common/CoreCommon.hpp
		Spherical.hpp
		StateInputManager.hpp
		Mat.hpp
		MatHelpers.hpp
		Quaternion.hpp
		Serializable.hpp
		Memory.hpp
		)

add_library(${MODULE_NAME} OBJECT ${SOURCES} ${HEADERS})

#target_precompile_headers(${MODULE_NAME} PUBLIC Kernel_pch.hpp)
set_target_properties(${MODULE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${MODULE_NAME} PROPERTIES FOLDER "MesumCore")
set_target_properties(${MODULE_NAME} PROPERTIES UNITY_BUILD ON)

target_include_directories(${MODULE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${MODULE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../Externals)

if (ENABLE_FBX)
	set(SOURCES ${SOURCES} FbxImporter.cpp)
	set(HEADERS ${HEADERS} FbxImporter.hpp)

	set(FBX_DIRECTORY "${EXTERNALS_SRC_DIR}/FBX SDK 2020.0.1")
	set(FBX_INCLUDE_DIRECTORY "${FBX_DIRECTORY}/include")
	set(FBX_LIBS_DIRECTORY "${FBX_DIRECTORY}/lib/vs2017/x64/$<CONFIGURATION>")

	message(STATUS "${FBX_DIRECTORY}")
	message(STATUS "${FBX_INCLUDE_DIRECTORY}")
	message(STATUS "${FBX_LIBS_DIRECTORY}")

	target_include_directories(${MODULE_NAME} PUBLIC ${FBX_INCLUDE_DIRECTORY})
	target_link_libraries(${MODULE_NAME}
			${FBX_LIBS_DIRECTORY}/libfbxsdk-md.lib
			${FBX_LIBS_DIRECTORY}/libxml2-md.lib
			${FBX_LIBS_DIRECTORY}/zlib-md.lib)
endif ()

if (BM_DYNAMIC_LINK)
	target_compile_definitions(${MODULE_NAME} PRIVATE _M_CORE_EXPORT)
endif ()