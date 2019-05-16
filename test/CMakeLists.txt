cmake_minimum_required(VERSION 3.1)
project(libkafka_asio_test)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../cmake/Modules")

#Give a pointer on where to find our installed pakages.
list(APPEND CMAKE_PREFIX_PATH "${ZLIB_HOME}")
list(APPEND CMAKE_PREFIX_PATH "${SNAPPY_HOME}")
list(APPEND CMAKE_PREFIX_PATH "${Boost_DIR}")

find_package(Boost REQUIRED) 
find_package(ZLIB REQUIRED)
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

#if(UNIX)
#target_link_libraries(${PROJECT_NAME} pthread)
#endif()

if(ADD_TESTS)
    enable_testing()
    #ADD_TESTS(${PROJECT_NAME} "" ${${PROJECT_NAME}_SOURCES})
endif()