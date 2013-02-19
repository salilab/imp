# duh
set(CGAL_DONT_OVERRIDE_CMAKE_FLAGS True)

include(FindCGAL)


if (${CGAL_FOUND})
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/CGAL" "ok=True")
include(${CGAL_USE_FILE})

else(${CGAL_FOUND})
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/CGAL" "ok=False")

endif(${CGAL_FOUND})

# reset build type
