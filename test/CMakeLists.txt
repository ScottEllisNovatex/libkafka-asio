cmake_minimum_required(VERSION 3.1)
project(libkafka_asio_test)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../cmake/Modules")

#Give a pointer on where to find our installed pakages.
list(APPEND CMAKE_PREFIX_PATH "${ZLIB_HOME}")
list(APPEND CMAKE_PREFIX_PATH "${SNAPPY_HOME}")
list(APPEND CMAKE_PREFIX_PATH "${Boost_DIR}")

#Have to stuff around to get the correct ZLIB libraries, the find package always seems to come back with the DLL version.
if (BUILD_SHARED_LIBS)
	find_package(ZLIB REQUIRED)
else()
	# Handle appveyor only building the single target.
	set( ZLIB_LIBRARIES "")
	
	if(UNIX)	
		find_library(ZLIB_STATIC libz.a PATHS "${ZLIB_HOME}" NO_SYSTEM_ENVIRONMENT_PATH)	##static linux equivalent.. (.so dynamic). We want to use our version, not one somewhere else in the system
		
		if (NOT (${ZLIB_STATIC} STREQUAL "ZLIB_STATIC-NOTFOUND"))
			list( APPEND ZLIB_LIBRARIES  ${ZLIB_STATIC})
		endif()
	else()
		find_library(ZLIB_STATIC zlibstatic PATHS "${ZLIB_HOME}" NO_SYSTEM_ENVIRONMENT_PATH)	#We want to use our version, not one somewhere else in the system
		find_library(ZLIB_STATICD zlibstaticd PATHS "${ZLIB_HOME}" NO_SYSTEM_ENVIRONMENT_PATH)
	
		if (NOT (${ZLIB_STATIC} STREQUAL "ZLIB_STATIC-NOTFOUND"))
			list( APPEND ZLIB_LIBRARIES optimized ${ZLIB_STATIC})
		endif()
		if (NOT (${ZLIB_STATICD} STREQUAL "ZLIB_STATICD-NOTFOUND"))
			list( APPEND ZLIB_LIBRARIES debug ${ZLIB_STATICD})
		endif()
	endif()
	set(ZLIB_INCLUDE_DIR "${ZLIB_HOME}/include")
endif()

find_package(Boost REQUIRED) 
find_package(Snappy REQUIRED)

message( "ZLIB_LIBRARIES - ${ZLIB_LIBRARIES}")
message( "SNAPPY_LIBRARIES - ${SNAPPY_LIBRARIES}")

include_directories(
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/../lib"
	"${ASIO_HOME}/include"    
    "${ZLIB_INCLUDE_DIR}"
    "${SNAPPY_INCLUDE_DIR}"
    "${Boost_INCLUDE_DIRS}")
	 
file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/*.h" "${PROJECT_SOURCE_DIR}/../lib/*.h")


add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})
target_link_libraries(${PROJECT_NAME} ${ZLIB_LIBRARIES} ${SNAPPY_LIBRARIES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

if(UNIX)
	target_link_libraries(${PROJECT_NAME} pthread)
endif()

message("Testing Enabled - test project ${PROJECT_NAME}")

enable_testing()
add_test( NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${INSTALLDIR_BINS})