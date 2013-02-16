message(STATUS "IMP.cgal config")

include(FindCGAL)

if (${CGAL_FOUND})
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/CGAL" "ok=True")
include(${CGAL_USE_FILE})
else(${CGAL_FOUND})
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/CGAL" "ok=False")
endif(${CGAL_FOUND})
