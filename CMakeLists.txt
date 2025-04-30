cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-Ofast")
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# Get the SDK
set(SDK $ENV{PLAYDATE_SDK_PATH})

# Game Name Customization
set(PLAYDATE_GAME_NAME nightwatch)
set(PLAYDATE_GAME_DEVICE ${PLAYDATE_GAME_NAME}_DEVICE)

project(${PLAYDATE_GAME_NAME} C ASM)

file(GLOB_RECURSE SOURCE_FILES src/*.c)

if (TOOLCHAIN STREQUAL "armgcc")
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
    set(MINIGL_TOOLS OFF CACHE BOOL "")
    set(MINIGL_PNG OFF CACHE BOOL "")

	set(MCFLAGS -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -D__FPU_USED=1 -mfp16-format=ieee)

	add_compile_options(${MCFLAGS})
	add_compile_options(-falign-functions=16 -fomit-frame-pointer)
	add_compile_options(-gdwarf-2)
	add_compile_options(-fverbose-asm)
	add_compile_options(-ffunction-sections -fdata-sections)
	add_compile_options(-mword-relocations -fno-common)

    add_link_options(-specs=nosys.specs)

    add_subdirectory(libs/minigl)

    set(TARGET_NAME ${PLAYDATE_GAME_DEVICE})
    add_executable(${TARGET_NAME}
        ${SOURCE_FILES}
        main.c)

else()
    set(BUILD_SHARED_LIBS ON CACHE BOOL "")
    add_subdirectory(libs/minigl)

    #add_definitions(-DMINIGL_DEBUG_PERF)
    add_definitions(-DDEBUG_MINIMAP)

    set(TARGET_NAME ${PLAYDATE_GAME_NAME})
    add_library(${TARGET_NAME} SHARED 
        ${SOURCE_FILES}
        main.c)

    # add_executable(map_viewer
    #     ${SOURCE_FILES}
    #     tools/map-viewer/main.c)
    # include_directories(map_viewer include)
    # target_link_libraries(map_viewer PUBLIC minigl)

    add_custom_target(sim
        DEPENDS ${TARGET_NAME}
        COMMAND ${SDK}/bin/PlaydateSimulator ${PROJECT_SOURCE_DIR}/${TARGET_NAME}.pdx
        COMMENT "Starting Simulator...")

    add_custom_target(valgrind
        DEPENDS ${TARGET_NAME}
        COMMAND valgrind 
            --leak-check=full --track-origins=yes --show-leak-kinds=all
            ${SDK}/bin/PlaydateSimulator ${PROJECT_SOURCE_DIR}/${TARGET_NAME}.pdx 2>&1 | tee -a valgrind.log
        COMMENT "Starting Valgrind...")
endif()

include_directories(${TARGET_NAME} include)
target_link_libraries(${TARGET_NAME} PUBLIC minigl)

include(cmake/playdate_game.cmake)
