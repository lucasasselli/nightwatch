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
set(PLAYDATE_GAME_DEVICE hello_world_DEVICE)

project(${PLAYDATE_GAME_NAME} C ASM)

file(GLOB SOURCE_FILES src/*.c)
file(GLOB MINIGL_FILES libs/minigl/src/*.c)

include_directories(${PLAYDATE_GAME_NAME} include)
include_directories(${PLAYDATE_GAME_NAME} libs/minigl/include)

if (TOOLCHAIN STREQUAL "armgcc")
	add_executable(${PLAYDATE_GAME_DEVICE} 
        ${MINIGL_FILES}
        ${SOURCE_FILES}
        main.c)
else()
    add_library(${PLAYDATE_GAME_NAME} SHARED 
        ${MINIGL_FILES}
        ${SOURCE_FILES}
        main.c)

    add_custom_target(sim
        DEPENDS ${PLAYDATE_GAME_NAME}
        COMMAND ${SDK}/bin/PlaydateSimulator ${PROJECT_SOURCE_DIR}/${PLAYDATE_GAME_NAME}.pdx
        COMMENT "Starting Simulator...")

    add_custom_target(valgrind
        DEPENDS ${PLAYDATE_GAME_NAME}
        COMMAND valgrind ${SDK}/bin/PlaydateSimulator ${PROJECT_SOURCE_DIR}/${PLAYDATE_GAME_NAME}.pdx
        COMMENT "Starting Valgrind...")
endif()


include(${SDK}/C_API/buildsupport/playdate_game.cmake)
