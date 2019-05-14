cmake_minimum_required(VERSION 2.6)
project(libkafka_asio)

option(USE_ASIO_SUBMODULE "Use git submodule to download asio header library" ON)
option(USE_ZLIB_SUBMODULE "Use git submodule to download zlib code/library" ON)
option(USE_SNAPPY_SUBMODULE "Use git submodule to download snappy code/library" ON)
option(ADD_TESTS "Build the tests" ON)

# Only when we use asio standalone.. 
add_definitions(-DASIO_STANDALONE) # required for ASIO in C++11 only mode

# Platform configuration
if(WIN32)
	set(CMAKE_SUPPRESS_REGENERATION true)	# removes the ZERO_CHECK project for VS, which rebuilds on CMake file change.

	# for ASIO
	add_definitions(-D_WIN32_WINNT=0x0502)		# Windows Server 2003 and up
	add_definitions(-DBOOST_ALL_NO_LIB)				# Otherwise, boost trys to tell the linker where the libraries are.
	add_definitions(-DASIO_HAS_STD_SYSTEM_ERROR)
	add_definitions(-DVC_EXTRALEAN)				# This and next one reduce size of windows header files.
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	
	# Auto generate def files that export all symbols of STATIC libraries
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
	
	# UNICODE support
	add_definitions(-DUNICODE -D_UNICODE)
	
	# Postfix for debug builds
	SET(CMAKE_DEBUG_POSTFIX "d")
	
	#min max macro rubbish
	add_definitions(-DNOMINMAX)

	# Install directories
	set(INSTALLDIR_BINS ".")
	set(INSTALLDIR_INCLUDES "include")
	set(INSTALLDIR_LIBS "lib")
	set(INSTALLDIR_MODULES ".")
endif()
if(UNIX)
	set(DL dl)
	set(RPATH_BASE $ORIGIN)

	# Install directories
	set(INSTALLDIR_BINS "bin")
	set(INSTALLDIR_INCLUDES "include")
	set(INSTALLDIR_LIBS "lib")
	set(INSTALLDIR_MODULES "plugin")
endif()
if(APPLE)
	set(CMAKE_MACOSX_RPATH 1)
	set(RPATH_BASE @executable_path)

	# Install directories
	set(INSTALLDIR_BINS "bin")
	set(INSTALLDIR_INCLUDES "include")
	set(INSTALLDIR_LIBS "lib")
	set(INSTALLDIR_MODULES "plugin")
endif()

# Compiler configuration


if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	# using Visual Studio C++
	message("CMAKE configuring opendatacon for Visual Studio C++" )
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /MP")

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
	# using Intel C++
	message(WARNING "opendatacon has not been tested using Intel compiler" )
	
else()#if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
	# using Clang or GNU compilers
	message("-- configuring opendatacon for  Clang or GNU compilers" )
	if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		message("using special options for GNU compiler")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --no-gnu-unique")
	endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -pedantic")
	option(STATIC_LIBSTDC++ "Link libstdc++ statically" ON)
	if(STATIC_LIBSTDC++)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
	endif()

	# different release and debug flags
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g")

	if(COVERAGE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
	endif()

	if (WERROR)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
	endif()

	#package the c++ standard library
	option(PACKAGE_LIBSTDC++ "Package libstdc++ in c-pack installer" OFF)
	if(PACKAGE_LIBSTDC++)
		execute_process(
			COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++.so.6
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			OUTPUT_VARIABLE std_lib
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		#hide (in non-advanced mode) the library paths in cmake guis since they are derrived
		mark_as_advanced(FORCE std_lib)
		get_filename_component(_resolvedFile "${std_lib}" REALPATH) #check for symolic link
		if(${_resolvedFile} STREQUAL ${std_lib})
			#not a link - just install file
			install(FILES "${std_lib}" DESTINATION ${INSTALLDIR_LIBS} CONFIGURATIONS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
		else()
			#is a link -install link and file
			install(FILES "${std_lib}" "${_resolvedFile}" DESTINATION ${INSTALLDIR_LIBS} CONFIGURATIONS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
		endif()
	endif()
endif()

find_package(Threads REQUIRED)
if(CMAKE_USE_PTHREADS_INIT)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# Locate SNAPPY
option(USE_SNAPPY_SUBMODULE "Use git submodule to download and build snappy libraries" ON)
set(SNAPPY_HOME_INSTRUCTIONS "Choose the location of snappy where it was installed - there should be lib and include directories within")
if(USE_SNAPPY_SUBMODULE)
	set(SNAPPY_SOURCE "${CMAKE_SOURCE_DIR}/submodules/SNAPPY")
	set(SNAPPY_BUILD "${CMAKE_BINARY_DIR}/submodules/SNAPPY")
	mark_as_advanced(FORCE SNAPPY_SOURCE)
	mark_as_advanced(FORCE SNAPPY_BUILD)
	set(SNAPPY_HOME "${SNAPPY_BUILD}/install" CACHE PATH ${SNAPPY_HOME_INSTRUCTIONS} FORCE)
	set(
		SNAPPY_CMAKE_OPTS
			-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
			-DCMAKE_INSTALL_PREFIX=${SNAPPY_HOME}/
			-DCMAKE_DEBUG_POSTFIX=${CMAKE_DEBUG_POSTFIX}
			-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
			-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
		CACHE STRING "cmake options to use when building SNAPPY submodule"
	)

	if(NOT EXISTS "${SNAPPY_SOURCE}/.git")
		execute_process(COMMAND git submodule update --init -- submodules/snappy WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
	endif()
	if(NOT EXISTS "${SNAPPY_BUILD}")
		file(MAKE_DIRECTORY "${SNAPPY_BUILD}")
	endif()
	if(DEFINED ${CMAKE_GENERATOR_PLATFORM})
		set(PLATFORM_OPT "-A${CMAKE_GENERATOR_PLATFORM}")
	elseif(DEFINED ${CMAKE_VS_PLATFORM_NAME})
		set(PLATFORM_OPT "-A${CMAKE_VS_PLATFORM_NAME}")
	else()
		set(PLATFORM_OPT "")
	endif()
	message("${CMAKE_COMMAND} ${SNAPPY_CMAKE_OPTS} -G${CMAKE_GENERATOR} ${PLATFORM_OPT} ${SNAPPY_SOURCE}")
	execute_process(
		COMMAND ${CMAKE_COMMAND} ${SNAPPY_CMAKE_OPTS} -G${CMAKE_GENERATOR} ${PLATFORM_OPT} ${SNAPPY_SOURCE}
		WORKING_DIRECTORY "${SNAPPY_BUILD}"
		RESULT_VARIABLE EXEC_RESULT
	)
	if(EXEC_RESULT)
		message( FATAL_ERROR "Failed to run cmake for SNAPPY submodule, exiting")
	endif()
	set(CONFIGS "${CMAKE_BUILD_TYPE}")
	if("${CMAKE_CONFIGURATION_TYPES}" MATCHES ".*Rel.*")
		list(APPEND CONFIGS "Release")
	endif()
	if("${CMAKE_CONFIGURATION_TYPES}" MATCHES ".*Deb.*")
		list(APPEND CONFIGS "Debug")
	endif()
	foreach(CONF ${CONFIGS})
		message("Building SNAPPY dependency")
		execute_process(
			COMMAND ${CMAKE_COMMAND} --build ${SNAPPY_BUILD} --config ${CONF} --target install
			WORKING_DIRECTORY "${SNAPPY_BUILD}"
			RESULT_VARIABLE EXEC_RESULT
		)
		if(EXEC_RESULT)
			message( FATAL_ERROR "Failed to build SNAPPY submodule, exiting")
		endif()
	endforeach()
	add_custom_target( build_SNAPPY
		WORKING_DIRECTORY "${SNAPPY_BUILD}"
		COMMAND ${CMAKE_COMMAND} --build ${SNAPPY_BUILD} --config $<CONFIG> --target install
	)
else()
	if(DEFINED SNAPPY_HOME)
		set(SNAPPY_HOME ${SNAPPY_HOME} CACHE PATH ${SNAPPY_HOME_INSTRUCTIONS})
	else()
		set(SNAPPY_HOME "/usr" CACHE PATH ${SNAPPY_HOME_INSTRUCTIONS})
	endif()
endif()

set(ASIO_HOME_INSTRUCTIONS "Choose the location of asio header library: where it was installed - there should be an include directory within")
if(USE_ASIO_SUBMODULE)
	set(ASIO_HOME "${CMAKE_SOURCE_DIR}/submodules/asio/asio" CACHE PATH ${ASIO_HOME_INSTRUCTIONS} FORCE)
	if(NOT EXISTS "${ASIO_HOME}/.git")
		execute_process(COMMAND git submodule update --init -- submodules/asio
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
	endif()
else()
	if(DEFINED ASIO_HOME)
		set(ASIO_HOME ${ASIO_HOME} CACHE PATH ${ASIO_HOME_INSTRUCTIONS})
	else()
		set(ASIO_HOME "/usr" CACHE PATH ${ASIO_HOME_INSTRUCTIONS})
	endif()
endif()

add_subdirectory("${PROJECT_SOURCE_DIR}/examples")

if (ADD_TESTS)
	enable_testing()
	add_subdirectory("${PROJECT_SOURCE_DIR}/test")
endif()



