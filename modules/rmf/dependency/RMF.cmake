message(STATUS "Building internal RMF")

set(RMF_BINARY_DIR ${PROJECT_BINARY_DIR}/src/dependency/RMF)

add_subdirectory(${PROJECT_SOURCE_DIR}/modules/rmf/dependency/RMF ${RMF_BINARY_DIR})

file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/RMF" "ok=True")

set(RMF_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/modules/rmf/dependency/RMF/include ${RMF_BINARY_DIR}/include  CACHE INTERNAL "" FORCE)
set(RMF_SWIG_PATH ${PROJECT_SOURCE_DIR}/modules/rmf/dependency/RMF/swig CACHE INTERNAL "" FORCE)
set(RMF_LIBRARIES RMF CACHE INTERNAL "" FORCE)