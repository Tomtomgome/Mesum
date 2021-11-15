set(APP_NAME WorldExplorer)

project(${APP_NAME} VERSION 1.0.0 DESCRIPTION "World Explorer")

set(SOURCES
		WeMain.cpp WeMain.hpp
		Camera.cpp Camera.hpp
		CameraOrbitController.cpp CameraOrbitController.hpp)

add_executable(${APP_NAME} ${BM_APP_WINDOWED} ${SOURCES})

target_link_libraries(${APP_NAME} PUBLIC MesumCore)
target_link_libraries(${APP_NAME} PUBLIC MesumGraphics)

set_target_properties(${APP_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

# TODO COPY DXIL.DLL NEXT TO THE EXE