
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)

set(MESUM_CORE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/MesumCore)
set(MESUM_GRAPHICS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/MesumGraphics)

add_subdirectory(MesumCore)
add_subdirectory(MesumGraphics)
