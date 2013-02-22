message(STATUS "Building internal RMF")

set(RMF_BINARY_DIR ${PROJECT_BINARY_DIR}/src/dependency/RMF)

add_subdirectory(${PROJECT_SOURCE_DIR}/modules/rmf/dependency/RMF ${RMF_BINARY_DIR})

set(RMF_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/modules/rmf/dependency/RMF/include ${RMF_BINARY_DIR}/include  CACHE INTERNAL "" FORCE)
set(RMF_SWIG_PATH ${PROJECT_SOURCE_DIR}/modules/rmf/dependency/RMF/swig CACHE INTERNAL "" FORCE)
set(RMF_LIBRARIES RMF CACHE INTERNAL "" FORCE)

file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/RMF" "ok=True
includepath=\"RMF_INCLUDE_PATH}\"
swigpath=\"RMF_SWIG_PATH}\"
libpath=\"RMF_SWIG_PATH}\"
")

set(ENV{PYTHONPATH} ${PROJECT_BINARY_DIR}/src/dependency/RMF/:$ENV{PYTHONPATH})
set(ENV{PATH} ${PROJECT_BINARY_DIR}/src/dependency/RMF/:$ENV{PATH})

message(STATUS "PATH is now "$ENV{PATH})
message(STATUS "PYTHONPATH is now "$ENV{PYTHONPATH})