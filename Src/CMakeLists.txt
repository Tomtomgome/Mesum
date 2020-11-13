cmake_minimum_required(VERSION 3.12)

add_subdirectory(Logger)
add_subdirectory(Kernel)
add_subdirectory(Input)
add_subdirectory(Simd)
add_subdirectory(Math)
add_subdirectory(Application)

add_subdirectory(PlatUnix)
add_subdirectory(PlatWindows)

add_subdirectory(CrossPlatform)

add_subdirectory(DX12Renderer)

