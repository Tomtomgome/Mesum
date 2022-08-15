set(APP_NAME WorldExplorer)

project(${APP_NAME} VERSION 1.0.0 DESCRIPTION "World Explorer")

set(SOURCES
		WeMain.cpp WeMain.hpp
		Camera.cpp Camera.hpp
		CameraOrbitController.cpp CameraOrbitController.hpp
		SrtmHm.cpp SrtmHm.hpp
		HgtPosition.cpp HgtPosition.hpp)

add_executable(${APP_NAME} ${BM_APP_WINDOWED} ${SOURCES})

target_link_libraries(${APP_NAME} PUBLIC MesumCore)
target_link_libraries(${APP_NAME} PUBLIC MesumGraphics)

set_target_properties(${APP_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

# TODO MOVE DXIL IN DX12RENDERER
add_custom_command(TARGET ${APP_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		"${EXTERNALS_SRC_DIR}/dxc_2021_04-20/bin/x64/dxil.dll"
		$<TARGET_FILE_DIR:${APP_NAME}>)