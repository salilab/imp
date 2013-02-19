if(DEFINED CMAKE_BUILD_TYPE)
set(user_build_type ${CMAKE_BUILD_TYPE})
endif()

#message(STATUS "CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS})
set(old_flags ${CMAKE_CXX_FLAGS})

include(FindCGAL)


if (${CGAL_FOUND})
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/CGAL" "ok=True")
include(${CGAL_USE_FILE})

# clean up
if(DEFINED user_build_type)
message(STATUS "Restoring build type to " ${user_build_type} " from " ${CMAKE_BUILD_TYPE})
set(CMAKE_BUILD_TYPE ${user_build_type} CACHE STRING "Build type is one of Debug, Release" FORCE)
endif()

#message(STATUS "CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS ${old_flags} ${CMAKE_CXX_FLAGS} CACHE STRING "The compilation flags" FORCE)

else(${CGAL_FOUND})
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/CGAL" "ok=False")

endif(${CGAL_FOUND})

# reset build type
