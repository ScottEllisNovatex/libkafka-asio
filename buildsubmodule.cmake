#
#   Build a sub module in the configure phase. Used for submodules that do not change
#   and we need available (and installed) for the main build.
#   \param name             Name of the submodule
#	We assume all submodules are under the CMAKE_SOURCE_DIR
#
macro(build_submodule name modulehome cmake_opts)

	message("AusGrid Build Submodule - ${name}, Modulehome - ${modulehome}")
 	set(${name}_SOURCE "${CMAKE_SOURCE_DIR}/submodules/${name}")
	set(${name}_BUILD "${CMAKE_BINARY_DIR}/submodules/${name}")
	mark_as_advanced(FORCE ${name}_SOURCE)
	mark_as_advanced(FORCE ${name}_BUILD)
	
	set(
		${name}_CMAKE_OPTS
			-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
			-DCMAKE_INSTALL_PREFIX=${modulehome}
			-DCMAKE_DEBUG_POSTFIX=${CMAKE_DEBUG_POSTFIX}
			-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
			-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
			${cmake_opts}
		CACHE STRING "cmake options to use when building ${name} submodule"
	)
	message("OPTS - ${${name}_CMAKE_OPTS}")
	
	if(NOT EXISTS "${${name}_SOURCE}/.git")
		execute_process(COMMAND git submodule update --init -- submodules/${name} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
	endif()
	
	message("Build - ${${name}_BUILD}")
	
	if(NOT EXISTS "${${name}_BUILD}")
		file(MAKE_DIRECTORY "${${name}_BUILD}")
	endif()
	if(DEFINED ${CMAKE_GENERATOR_PLATFORM})
		set(PLATFORM_OPT "-A${CMAKE_GENERATOR_PLATFORM}")
	elseif(DEFINED ${CMAKE_VS_PLATFORM_NAME})
		set(PLATFORM_OPT "-A${CMAKE_VS_PLATFORM_NAME}")
	else()
		set(PLATFORM_OPT "")
	endif()
	message("${CMAKE_COMMAND} ${${name}_CMAKE_OPTS} -G${CMAKE_GENERATOR} ${PLATFORM_OPT} ${${name}_SOURCE}")
	execute_process(
		COMMAND ${CMAKE_COMMAND} ${${name}_CMAKE_OPTS} -G${CMAKE_GENERATOR} ${PLATFORM_OPT} ${${name}_SOURCE}
		WORKING_DIRECTORY "${${name}_BUILD}"
		RESULT_VARIABLE EXEC_RESULT
	)
	if(EXEC_RESULT)
		message( FATAL_ERROR "Failed to run cmake for ${name} submodule, exiting")
	endif()
	set(CONFIGS "${CMAKE_BUILD_TYPE}")
	if("${CMAKE_CONFIGURATION_TYPES}" MATCHES ".*Rel.*")
		list(APPEND CONFIGS "Release")
	endif()
	if("${CMAKE_CONFIGURATION_TYPES}" MATCHES ".*Deb.*")
		list(APPEND CONFIGS "Debug")
	endif()
	foreach(CONF ${CONFIGS})
		message("Building ${name} dependency")
		execute_process(
			COMMAND ${CMAKE_COMMAND} --build ${${name}_BUILD} --config ${CONF} --target install
			WORKING_DIRECTORY "${${name}_BUILD}"
			RESULT_VARIABLE EXEC_RESULT
		)
		if(EXEC_RESULT)
			message( FATAL_ERROR "Failed to build ${name} submodule, exiting")
		endif()
	endforeach()
	add_custom_target( build_${name}
		WORKING_DIRECTORY "${${name}_BUILD}"
		COMMAND ${CMAKE_COMMAND} --build ${${name}_BUILD} --config $<CONFIG> --target install
    )

endmacro(build_submodule)

