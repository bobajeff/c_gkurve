cmake_minimum_required(VERSION 3.0...3.25)
project(
	Geometry_Data # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)

#--------------CREATE_SURFACE
add_library(create_surface
create_surface.c
)
target_link_libraries(create_surface PRIVATE glfw wgpu_native)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_compile_definitions(create_surface PRIVATE
    WGPU_TARGET=WGPU_TARGET_WINDOWS
    )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_compile_definitions(create_surface PRIVATE
    WGPU_TARGET=WGPU_TARGET_MACOS
    )
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(create_surface PRIVATE
    WGPU_TARGET=WGPU_TARGET_LINUX_X11
    )
else()
    target_compile_definitions(create_surface PRIVATE
    WGPU_TARGET=NO_WGPU_TARGET
    )
endif()
#--------------FRAMEWORK
add_library(framework
framework.c
)
target_link_libraries(framework PRIVATE wgpu_native)
target_include_directories(framework PUBLIC ${CMAKE_SOURCE_DIR}/helpers)