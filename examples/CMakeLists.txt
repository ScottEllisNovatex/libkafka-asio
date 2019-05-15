cmake_minimum_required(VERSION 3.1)
project(libkafka_asio_examples)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../cmake/Modules")

#Give a pointer on where to find our installed pakages.
list(APPEND CMAKE_PREFIX_PATH "${ZLIB_HOME}")
list(APPEND CMAKE_PREFIX_PATH "${SNAPPY_HOME}")

# add_definitions("-DLIBKAFKAASIO_NO_COMPRESSION")
find_package(Boost REQUIRED) 
find_package(ZLIB REQUIRED)
find_package(Snappy REQUIRED)

include_directories(
        "${PROJECT_SOURCE_DIR}/../lib"
        "${Boost_INCLUDE_DIRS}"
		"${ASIO_HOME}/include" 		
		"${ZLIB_INCLUDE_DIR}"
		"${SNAPPY_INCLUDE_DIR}")

set(examples_LINK_LIBRARIES  ${ZLIB_LIBRARIES} ${SNAPPY_LINK_LIBRARY} ) 

#if (UNIX)
 #   list(APPEND examples_LINK_LIBRARIES pthread)
#endif ()

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES "${PROJECT_SOURCE_DIR}/../lib/*.h")

add_executable(fetch_cxx11 ${PROJECT_SOURCE_DIR}/fetch.cpp ${${PROJECT_NAME}_SOURCES})
target_link_libraries(fetch_cxx11 ${examples_LINK_LIBRARIES})
set_property(TARGET fetch_cxx11 PROPERTY CXX_STANDARD 11)

add_executable(produce_cxx11 ${PROJECT_SOURCE_DIR}/produce.cpp ${${PROJECT_NAME}_SOURCES})
target_link_libraries(produce_cxx11 ${examples_LINK_LIBRARIES})
set_property(TARGET produce_cxx11 PROPERTY CXX_STANDARD 11)

add_executable(metadata_cxx11 ${PROJECT_SOURCE_DIR}/metadata.cpp ${${PROJECT_NAME}_SOURCES})
target_link_libraries(metadata_cxx11 ${examples_LINK_LIBRARIES})
set_property(TARGET metadata_cxx11 PROPERTY CXX_STANDARD 11)

add_executable(offset_cxx11 ${PROJECT_SOURCE_DIR}/offset.cpp ${${PROJECT_NAME}_SOURCES})
target_link_libraries(offset_cxx11 ${examples_LINK_LIBRARIES})
set_property(TARGET offset_cxx11 PROPERTY CXX_STANDARD 11)

add_executable(offset_fetch_cxx11 ${PROJECT_SOURCE_DIR}/offset_fetch.cpp ${${PROJECT_NAME}_SOURCES})
target_link_libraries(offset_fetch_cxx11 ${examples_LINK_LIBRARIES})
set_property(TARGET offset_fetch_cxx11 PROPERTY CXX_STANDARD 11)
