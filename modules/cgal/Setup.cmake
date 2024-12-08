include(FindCGAL)

if (${CGAL_FOUND})
file(WRITE "${CMAKE_BINARY_DIR}/build_info/CGAL" "ok=True")
set(CGAL_DONT_OVERRIDE_CMAKE_FLAGS TRUE CACHE BOOL "Don't override flags")
# Ignore CGAL_CXX_FLAGS from previous cmake run, if any
set( CGAL_CXX_FLAGS "" CACHE INTERNAL "" FORCE)
if (DEFINED CGAL_USE_FILE)
  include(${CGAL_USE_FILE})
endif()
# Set "standard" include/link variables from CGAL's
set(CGAL_INCLUDE_PATH ${CGAL_INCLUDE_DIRS} CACHE INTERNAL "" FORCE)
set(CGAL_LINK_PATH ${CGAL_LIBRARIES_DIR} CACHE INTERNAL "" FORCE)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
message(STATUS "Updating CMAKE_CXX_FLAGS")
set( CGAL_CXX_FLAGS "-frounding-math" CACHE INTERNAL "" FORCE)
endif()

else(${CGAL_FOUND})
file(WRITE "${CMAKE_BINARY_DIR}/build_info/CGAL" "ok=False")

endif(${CGAL_FOUND})

# reset build type
