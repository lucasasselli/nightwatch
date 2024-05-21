cmake_minimum_required(VERSION 3.14)
project("PlaydateC")

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

# Get the SDK
set(SDK $ENV{PLAYDATE_SDK_PATH})

# Game Name Customization
set(PLAYDATE_GAME_NAME hello_world)
set(PLAYDATE_GAME_DEVICE ${PLAYDATE_GAME_NAME}_DEVICE)

project(${PLAYDATE_GAME_NAME} C ASM)

file(GLOB SOURCE_FILES src/*.c)
file(GLOB MINIGL_FILES libs/minigl/src/*.c)

set(ZLIB_ROOT, {PROJECT_SOURCE_DIR}/ext/zlib)

if (TOOLCHAIN STREQUAL "armgcc")
    set(SPNG_SHARED OFF CACHE BOOL "")
    set(CGLM_SHARED OFF CACHE BOOL "")

    add_compile_options(-mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -D__FPU_USED=1 -O3)

    set(TARGET_NAME ${PLAYDATE_GAME_DEVICE})
    add_executable(${TARGET_NAME}
        ${PROJECT_SOURCE_DIR}/ext/libspng/spng/spng.c
        ${MINIGL_FILES}
        ${SOURCE_FILES}
        main.c)

    target_link_options(${TARGET_NAME} PUBLIC -specs=nosys.specs)
else()
    set(BUILD_SHARED_LIBS ON CACHE BOOL "")

    add_definitions(-DDEBUG_PERF)

    set(TARGET_NAME ${PLAYDATE_GAME_NAME})
    add_library(${TARGET_NAME} SHARED 
        ${PROJECT_SOURCE_DIR}/ext/libspng/spng/spng.c
        ${MINIGL_FILES}
        ${SOURCE_FILES}
        main.c)

    add_custom_target(sim
        DEPENDS ${TARGET_NAME}
        COMMAND ${SDK}/bin/PlaydateSimulator ${PROJECT_SOURCE_DIR}/${TARGET_NAME}.pdx
        COMMENT "Starting Simulator...")

    add_custom_target(valgrind
        DEPENDS ${TARGET_NAME}
        COMMAND valgrind 
            --leak-check=full --track-origins=yes
            ${SDK}/bin/PlaydateSimulator ${PROJECT_SOURCE_DIR}/${TARGET_NAME}.pdx
        COMMENT "Starting Valgrind...")
endif()

add_definitions(-DDEBUG)
add_definitions(-DSPNG_USE_MINIZ)

add_subdirectory(ext/cglm)
add_subdirectory(ext/miniz)

include_directories(${TARGET_NAME} ext/libspng/spng)
include_directories(${TARGET_NAME} libs/minigl/include)
include_directories(${TARGET_NAME} ext/miniz)
include_directories(${TARGET_NAME} include)

target_link_libraries(${TARGET_NAME} PUBLIC cglm)
target_link_libraries(${TARGET_NAME} PUBLIC miniz)

include(${SDK}/C_API/buildsupport/playdate_game.cmake)
