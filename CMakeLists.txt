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
        GIT_TAG        73948e6c0f15b1053cf74b7c4e6b04fd36e97e29
)
FetchContent_MakeAvailable(glfw)

#--------------CGLM
FetchContent_Declare(
        cglm
        GIT_REPOSITORY https://github.com/recp/cglm
)
FetchContent_MakeAvailable(cglm)


#--------------LIBTESS2
FetchContent_Declare(
        libtess2
        GIT_REPOSITORY https://github.com/bobajeff/libtess2
)
FetchContent_MakeAvailable(libtess2)

#--------------FREETYPE
find_package(Freetype REQUIRED)

add_compile_definitions(RESOURCE_DIR="${CMAKE_SOURCE_DIR}/")

add_subdirectory(helpers)

#--------------MEMORY_ALLOC_HELPERS
add_library(memory_alloc_helpers
memory_alloc_helpers.c
)
target_link_libraries(memory_alloc_helpers PRIVATE cglm wgpu_native)
target_include_directories(memory_alloc_helpers PUBLIC ${CMAKE_SOURCE_DIR})

#--------------DRAW
add_library(draw
draw.c
)
target_link_libraries(draw PRIVATE wgpu_native cglm memory_alloc_helpers)
target_include_directories(draw PUBLIC ${CMAKE_SOURCE_DIR})

#--------------ATLAS
add_library(atlas
atlas.c
)
target_link_libraries(atlas PRIVATE wgpu_native cglm)
target_include_directories(atlas PUBLIC ${CMAKE_SOURCE_DIR})

#--------------LABEL
add_library(label
label.c
)
target_link_libraries(label PRIVATE wgpu_native cglm freetype draw)
target_include_directories(label PUBLIC ${CMAKE_SOURCE_DIR} ${FREETYPE_INCLUDE_DIRS})

#--------------TRIANGULATE_POLYGON
add_library(triangulate_polygon
triangulate_polygon.c
)
target_link_libraries(triangulate_polygon PRIVATE libtess2 cglm memory_alloc_helpers)
target_include_directories(triangulate_polygon PUBLIC ${CMAKE_SOURCE_DIR} ${FREETYPE_INCLUDE_DIRS} libtess2 cglm)

#--------------RESIZABLE_LABEL
add_library(resizable_label
resizable_label.c
)
target_link_libraries(resizable_label PRIVATE wgpu_native cglm freetype memory_alloc_helpers triangulate_polygon)
target_include_directories(resizable_label PUBLIC ${CMAKE_SOURCE_DIR} ${FREETYPE_INCLUDE_DIRS})

#--------------GKURVE
add_executable(gkurve
main.c
)
target_link_libraries(gkurve PRIVATE glfw wgpu_native create_surface framework cglm draw atlas label resizable_label)


