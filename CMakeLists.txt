cmake_minimum_required(VERSION 3.2)
project(libkafka_asio)

include(buildsubmodule.cmake)

option(USE_ASIO_SUBMODULE "Use git submodule to download asio header library" ON)
option(USE_ZLIB_SUBMODULE "Use git submodule to download zlib code/library" ON)
option(USE_SNAPPY_SUBMODULE "Use git submodule to download snappy code/library" ON)
option(LIBKAFKA_ASIO_BUILD_TESTS "Build the libkafka-asio catch2 based tests" ON)

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

set( Boost_DIR "${CMAKE_SOURCE_DIR}/submodules/boost_1_70_0" )

# Locate SNAPPY
set(SNAPPY_HOME_INSTRUCTIONS "Choose the location of Snappy where it was installed - there should be lib and include directories within")
if(USE_SNAPPY_SUBMODULE)
	set(SNAPPY_HOME "${CMAKE_BINARY_DIR}/submodules/Snappy/install" CACHE PATH ${SNAPPY_HOME_INSTRUCTIONS} FORCE)
	
	build_submodule(snappy ${SNAPPY_HOME} "-DSNAPPY_BUILD_TESTS=OFF")
	
	#add_subdirectory(submodules/SNAPPY)	# this would build at build time which does not work well for VS
else()
	if(DEFINED SNAPPY_HOME)
		set(SNAPPY_HOME ${SNAPPY_HOME} CACHE PATH ${SNAPPY_HOME_INSTRUCTIONS})
	else()
		set(SNAPPY_HOME "/usr" CACHE PATH ${SNAPPY_HOME_INSTRUCTIONS})
	endif()
endif()
message("Snappy_home - ${SNAPPY_HOME}")

# Locate ZLIB
set(ZLIB_HOME_INSTRUCTIONS "Choose the location of ZLIB where it was installed - there should be lib and include directories within")
if(USE_ZLIB_SUBMODULE)
	set(ZLIB_HOME "${CMAKE_BINARY_DIR}/submodules/ZLIB/install" CACHE PATH ${ZLIB_HOME_INSTRUCTIONS} FORCE)
	
	build_submodule(zlib ${ZLIB_HOME} "")
	
else()
	if(DEFINED ZLIB_HOME)
		set(ZLIB_HOME ${ZLIB_HOME} CACHE PATH ${ZLIB_HOME_INSTRUCTIONS})
	else()
		set(ZLIB_HOME "/usr" CACHE PATH ${ZLIB_HOME_INSTRUCTIONS})
	endif()
endif()
message("Zlib_home - ${ZLIB_HOME}")


#ASIO
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

if (LIBKAFKA_ASIO_BUILD_TESTS)
	enable_testing()
	add_subdirectory("${PROJECT_SOURCE_DIR}/test")
endif()



