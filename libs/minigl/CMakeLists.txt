cmake_minimum_required(VERSION 3.14)

project(minigl C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

option(MINIGL_SHARED "Build shared lib" ON)

file(GLOB MINIGL_FILES src/*.c)

if (NOT MINIGL_SHARED)
    set(SPNG_SHARED OFF CACHE BOOL "")
    set(CGLM_SHARED OFF CACHE BOOL "")

    add_library(minigl STATIC
        ${PROJECT_SOURCE_DIR}/ext/libspng/spng/spng.c
        ${MINIGL_FILES})
else()
    set(BUILD_SHARED_LIBS ON CACHE BOOL "")

    add_library(minigl SHARED 
        ${PROJECT_SOURCE_DIR}/ext/libspng/spng/spng.c
        ${MINIGL_FILES})
endif()


add_definitions(-DSPNG_USE_MINIZ)

add_subdirectory(ext/cglm)
add_subdirectory(ext/miniz)

target_include_directories(minigl PUBLIC ext/libspng/spng)
target_include_directories(minigl PUBLIC ext/miniz)
target_include_directories(minigl PUBLIC include)

target_link_libraries(minigl PUBLIC cglm)
target_link_libraries(minigl PUBLIC miniz)
target_link_libraries(minigl PUBLIC m)
