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

project(${TARGET_NAME} C ASM)

file(GLOB SOURCE_FILES src/*.c)
file(GLOB MINIGL_FILES libs/minigl/src/*.c)


if (TOOLCHAIN STREQUAL "armgcc")
    set(TARGET_NAME ${PLAYDATE_GAME_NAME}_DEVICE)
	add_executable(${PLAYDATE_GAME_DEVICE} 
        ${MINIGL_FILES}
        ${SOURCE_FILES}
        main.c)
else()
    set(TARGET_NAME ${PLAYDATE_GAME_NAME})
    add_library(${TARGET_NAME} SHARED 
        ${MINIGL_FILES}
        ${SOURCE_FILES}
        main.c)

    add_custom_target(sim
        DEPENDS ${TARGET_NAME}
        COMMAND ${SDK}/bin/PlaydateSimulator ${PROJECT_SOURCE_DIR}/${TARGET_NAME}.pdx
        COMMENT "Starting Simulator...")

    add_custom_target(valgrind
        DEPENDS ${TARGET_NAME}
        COMMAND valgrind ${SDK}/bin/PlaydateSimulator ${PROJECT_SOURCE_DIR}/${TARGET_NAME}.pdx
        COMMENT "Starting Valgrind...")
endif()

add_definitions(-DDEBUG)

add_subdirectory(ext/cglm)
add_subdirectory(ext/libspng)

include_directories(${TARGET_NAME} include)
include_directories(${TARGET_NAME} libs/minigl/include)

target_link_libraries(${TARGET_NAME} PRIVATE cglm)
target_link_libraries(${TARGET_NAME} PRIVATE spng)

include(${SDK}/C_API/buildsupport/playdate_game.cmake)
