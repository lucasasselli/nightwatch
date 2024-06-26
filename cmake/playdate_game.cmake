#
# CMake include file for Playdate games
#
cmake_minimum_required(VERSION 3.19)

include(cmake/playdate.cmake)

set(BUILD_SUB_DIR "")

if (TOOLCHAIN STREQUAL "armgcc")
	set_property(TARGET ${PLAYDATE_GAME_DEVICE} PROPERTY OUTPUT_NAME "${PLAYDATE_GAME_DEVICE}.elf")

	add_custom_command(
		TARGET ${PLAYDATE_GAME_DEVICE} POST_BUILD
		COMMAND ${CMAKE_STRIP} --strip-unneeded -R .comment -g
		${PLAYDATE_GAME_DEVICE}.elf
		-o ${CMAKE_CURRENT_SOURCE_DIR}/game/pdex.elf
	)

	add_custom_command(
		TARGET ${PLAYDATE_GAME_DEVICE} POST_BUILD
        COMMAND ${CMAKE_OBJDUMP} --visualize-jumps --source ${PLAYDATE_GAME_DEVICE}.elf > ${PLAYDATE_GAME_DEVICE}_dump.txt
	)

	set_property(
		TARGET ${PLAYDATE_GAME_DEVICE} APPEND PROPERTY ADDITIONAL_CLEAN_FILES
		${CMAKE_CURRENT_SOURCE_DIR}/game/pdex.elf
		)

	set_property(
		TARGET ${PLAYDATE_GAME_DEVICE} APPEND PROPERTY ADDITIONAL_CLEAN_FILES
		${CMAKE_CURRENT_SOURCE_DIR}/${PLAYDATE_GAME_NAME}.pdx
		)

	add_custom_command(
		TARGET ${PLAYDATE_GAME_DEVICE} POST_BUILD
		COMMAND ${PDC} game ${PLAYDATE_GAME_NAME}.pdx
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

    add_custom_target(release
        DEPENDS ${TARGET_NAME}
        COMMAND zip -r ${PLAYDATE_GAME_NAME}.pdx.zip ${PLAYDATE_GAME_NAME}.pdx
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

else ()

	set(LIB_EXT "")

	if (MSVC)

		set(LIB_EXT "dll")

		if(${CMAKE_GENERATOR} MATCHES "Visual Studio*" )
			set(BUILD_SUB_DIR $<CONFIG>/)
			file(TO_NATIVE_PATH ${SDK}/bin/PlaydateSimulator.exe SIMPATH)
			file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${PLAYDATE_GAME_NAME}.pdx SIMGAMEPATH)
			set_property(TARGET ${PLAYDATE_GAME_NAME} PROPERTY VS_DEBUGGER_COMMAND ${SIMPATH})
			set_property(TARGET ${PLAYDATE_GAME_NAME} PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "\"${SIMGAMEPATH}\"")
		endif()

		add_custom_command(
			TARGET ${PLAYDATE_GAME_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_BINARY_DIR}/${BUILD_SUB_DIR}${PLAYDATE_GAME_NAME}.dll
			${CMAKE_CURRENT_SOURCE_DIR}/game/pdex.dll)

	elseif(APPLE)

		set(LIB_EXT "dylib")

		if(${CMAKE_GENERATOR} MATCHES "Xcode" )
			set(BUILD_SUB_DIR $<CONFIG>/)
			set_property(TARGET ${PLAYDATE_GAME_NAME} PROPERTY XCODE_SCHEME_ARGUMENTS \"${CMAKE_CURRENT_SOURCE_DIR}/${PLAYDATE_GAME_NAME}.pdx\")
			set_property(TARGET ${PLAYDATE_GAME_NAME} PROPERTY XCODE_SCHEME_EXECUTABLE ${SDK}/bin/Playdate\ Simulator.app)
		endif()

		add_custom_command(
			TARGET ${PLAYDATE_GAME_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_BINARY_DIR}/${BUILD_SUB_DIR}lib${PLAYDATE_GAME_NAME}.dylib
			${CMAKE_CURRENT_SOURCE_DIR}/game/pdex.dylib)

	elseif(UNIX)

		set(LIB_EXT "so")

		add_custom_command(
			TARGET ${PLAYDATE_GAME_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_BINARY_DIR}/lib${PLAYDATE_GAME_NAME}.so
			${CMAKE_CURRENT_SOURCE_DIR}/game/pdex.so)

	elseif(MINGW)

		set(LIB_EXT "dll")

		add_custom_command(
			TARGET ${PLAYDATE_GAME_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_CURRENT_BINARY_DIR}/lib${PLAYDATE_GAME_NAME}.dll
			${CMAKE_CURRENT_SOURCE_DIR}/game/pdex.dll)
	else()
		message(FATAL_ERROR "Platform not supported!")
	endif()

	set_property(
		TARGET ${PLAYDATE_GAME_NAME} APPEND PROPERTY ADDITIONAL_CLEAN_FILES
		${CMAKE_CURRENT_SOURCE_DIR}/game/pdex.${LIB_EXT}
		)

	set_property(
		TARGET ${PLAYDATE_GAME_NAME} APPEND PROPERTY ADDITIONAL_CLEAN_FILES
		${CMAKE_CURRENT_SOURCE_DIR}/${PLAYDATE_GAME_NAME}.pdx
		)

	add_custom_command(
		TARGET ${PLAYDATE_GAME_NAME} POST_BUILD
		COMMAND ${PDC} ${CMAKE_CURRENT_SOURCE_DIR}/game
		${CMAKE_CURRENT_SOURCE_DIR}/${PLAYDATE_GAME_NAME}.pdx)

endif ()
