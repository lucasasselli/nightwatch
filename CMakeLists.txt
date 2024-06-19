cmake_minimum_required(VERSION 3.14)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-Ofast")
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

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
    set(MINIGL_SHARED OFF CACHE BOOL "")

	set(MCFLAGS -mthumb -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-sp-d16 -D__FPU_USED=1 -mfp16-format=ieee)

	add_compile_options(${MCFLAGS})
	add_compile_options(-falign-functions=16 -fomit-frame-pointer)
	add_compile_options(-gdwarf-2)
	add_compile_options(-fverbose-asm)
	add_compile_options(-ffunction-sections -fdata-sections)
	add_compile_options(-mword-relocations -fno-common)

    set(TARGET_NAME ${PLAYDATE_GAME_DEVICE})
    add_executable(${TARGET_NAME}
        ${SOURCE_FILES}
        main.c)

    target_link_options(${TARGET_NAME} PUBLIC -specs=nosys.specs)
else()
    add_definitions(-DMINIGL_DEBUG_PERF)
    add_definitions(-DDEBUG_MINIMAP)

    set(TARGET_NAME ${PLAYDATE_GAME_NAME})
    add_library(${TARGET_NAME} SHARED 
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
            ${SDK}/bin/PlaydateSimulator ${PROJECT_SOURCE_DIR}/${TARGET_NAME}.pdx 2>&1 | tee -a valgrind.log
        COMMENT "Starting Valgrind...")
endif()

add_definitions(-DDEBUG)

add_subdirectory(libs/minigl)

include_directories(${TARGET_NAME} include)

target_link_libraries(${TARGET_NAME} PUBLIC minigl)

include(cmake/playdate_game.cmake)
