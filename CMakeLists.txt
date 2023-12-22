cmake_minimum_required(VERSION 3.0...3.25)
project(
	c_gkurve # name of the project, which will also be the name of the visual studio solution if you use it
	VERSION 0.1.0 # any version number
	LANGUAGES CXX C # programming languages used by the project
)
# add_compile_options(-fsanitize=address)
# set (CMAKE_EXE_LINKER_FLAGS "-fsanitize=address")

#--------------WGPU_NATIVE
# send error for missing wgpu directory
if(EXISTS ${CMAKE_SOURCE_DIR}/wgpu)
  message( STATUS "wpu exists")
else()
  message( SEND_ERROR "\nYou will need to have wgpu-native. Get it from here: \nhttps://github.com/gfx-rs/wgpu-native/releases\nThen extract it to the root source directory and rename it to 'wgpu'\n" )
endif()
# assumes wgpu_native is downloaded and extracted to the root source directory and renamed 'wgpu' 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
add_library(wgpu_native STATIC IMPORTED)
find_library(WGPU_LIBRARY_LOC libwgpu_native.a libwgpu_native.lib PATHS 
"${CMAKE_SOURCE_DIR}/wgpu")
set_property(TARGET wgpu_native PROPERTY IMPORTED_LOCATION ${WGPU_LIBRARY_LOC})
target_include_directories(wgpu_native INTERFACE ${CMAKE_SOURCE_DIR}/wgpu)

include(FetchContent)

#--------------GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
)
FetchContent_MakeAvailable(glfw)

#--------------CGLM
FetchContent_Declare(
        cglm
        GIT_REPOSITORY https://github.com/recp/cglm
)
FetchContent_MakeAvailable(cglm)

add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/")

add_subdirectory(helpers)

#--------------DRAW
add_library(draw
draw.c
)
target_link_libraries(draw PRIVATE wgpu_native cglm)
target_include_directories(draw PUBLIC ${CMAKE_SOURCE_DIR})

#--------------ATLAS
add_library(atlas
atlas.c
)
target_link_libraries(atlas PRIVATE wgpu_native cglm)
target_include_directories(atlas PUBLIC ${CMAKE_SOURCE_DIR})

#--------------GKURVE
add_executable(gkurve
main.c
)
target_link_libraries(gkurve PRIVATE glfw wgpu_native create_surface framework cglm draw atlas)


