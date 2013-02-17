message(STATUS "IMP.cgal config")

if(DEFINED CMAKE_BUILD_TYPE)
set(user_build_type ${CMAKE_BUILD_TYPE})
endif()

include(FindCGAL)


if (${CGAL_FOUND})
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/CGAL" "ok=True")
include(${CGAL_USE_FILE})
else(${CGAL_FOUND})
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/CGAL" "ok=False")
endif(${CGAL_FOUND})

# reset build type
if(DEFINED user_build_type)
message(STATUS "Restoring build type to " ${user_build_type} " from " ${CMAKE_BUILD_TYPE})
set(CMAKE_BUILD_TYPE ${user_build_type} CACHE STRING "Build type is one of Debug, Release" FORCE)
endif()
