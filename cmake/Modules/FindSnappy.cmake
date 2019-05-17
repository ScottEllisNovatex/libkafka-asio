find_path(
  SNAPPY_INCLUDE_DIR
  NAMES snappy.h
  HINTS ${SNAPPY_ROOT_DIR}/include)

find_library(
  SNAPPY_LIBRARY
  NAMES snappy 
  HINTS ${SNAPPY_ROOT_DIR}/lib)
  
 find_library(
  SNAPPY_DEBUG_LIBRARY
  NAMES snappyd 
  HINTS ${SNAPPY_ROOT_DIR}/lib) 

#Handle the case where we are only building for only optimized or debug release. 

set( SNAPPY_LIBRARIES "")

if (NOT (${SNAPPY_LIBRARY} STREQUAL "SNAPPY_LIBRARY-NOTFOUND"))
	list( APPEND SNAPPY_LIBRARIES optimized "${SNAPPY_LIBRARY}")
endif()

if (NOT (${SNAPPY_DEBUG_LIBRARY} STREQUAL "SNAPPY_DEBUG_LIBRARY-NOTFOUND"))
	list( APPEND SNAPPY_LIBRARIES debug "${SNAPPY_DEBUG_LIBRARY}")
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  Snappy DEFAULT_MSG
  SNAPPY_LIBRARIES
  SNAPPY_INCLUDE_DIR)

mark_as_advanced(
  SNAPPY_ROOT_DIR
  SNAPPY_LIBRARIES
  SNAPPY_INCLUDE_DIR)
