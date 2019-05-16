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

# The snappy.cmake file gives us the path to each, choose the right one. Probably a better way to do this.
#set(SNAPPY_LINK_LIBRARY optimized ${SNAPPY_LIBRARY} debug ${SNAPPY_DEBUG_LIBRARY})

set( SNAPPY_LIBRARIES optimized "${SNAPPY_LIBRARY}" debug "${SNAPPY_DEBUG_LIBRARY}")

message( "FindSnappy - SNAPPY_LIBRARIES - ${SNAPPY_LIBRARY} - ${SNAPPY_DEBUG_LIBRARY}")
message( "FindSnappy - SNAPPY_LIBRARIES - ${SNAPPY_LIBRARIES}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  Snappy DEFAULT_MSG
  SNAPPY_LIBRARIES
  SNAPPY_INCLUDE_DIR)

mark_as_advanced(
  SNAPPY_ROOT_DIR
  SNAPPY_LIBRARIES
  SNAPPY_INCLUDE_DIR)
