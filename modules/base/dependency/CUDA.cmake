find_package(CUDA)

if (CUDA_VERSION_MAJOR EQUAL 5)
set(IMP_HAS_CUDA True CACHE INTERNAL "" FORCE)
set(CUDA_INCLUDE_PATH ${CUDA_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/modules/base/dependency/cuda/include CACHE INTERNAL "")
message(STATUS "CUDA_INCLUDE_PATH=${CUDA_INCLUDE_PATH}")

message(STATUS "NVCC_FLAGS=${CUDA_NVCC_FLAGS}")
set(CUDA_LIBRARIES ${CUDA_LIBRARIES} CACHE INTERNAL "" FORCE)

if(NOT DEFINED IMP_CUDA_SET_FLAGS)
set(IMP_CUDA_SET_FLAGS True CACHE INTERNAL "" FORCE)
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_20,code=sm_20 --compiler-options '-fPIC' --pre-include ${PROJECT_SOURCE_DIR}/modules/scratch/include/internal/undef_atomics_int128.h CACHE INTERNAL "" FORCE)
endif()

file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/CUDA" "ok=True
includepath=\"${CUDA_INCLUDE_PATH}\"
")


else()
message(STATUS "CUDA disabled (need version 5)")
set(IMP_HAS_CUDA False CACHE INTERNAL "" FORCE)
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/CUDA" "ok=False
")
endif()
