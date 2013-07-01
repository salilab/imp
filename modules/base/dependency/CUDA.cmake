find_package(CUDA)

if (DEFINED CUDA_VERSION_MAJOR)
set(CUDA_INCLUDE_PATH ${CUDA_INCLUDE_DIRS} CACHE INTERNAL "")
message(STATUS "CUDA_INCLUDE_PATH=${CUDA_INCLUDE_PATH}")

message(STATUS "NVCC_FLAGS=${CUDA_NVCC_FLAGS}")
set(CUDA_LIBRARIES ${CUDA_LIBRARIES} CACHE INTERNAL "" FORCE)

set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS};-gencode arch=compute_20,code=sm_20 --compiler-options '-fPIC' --pre-include ${PROJECT_SOURCE_DIR}/modules/scratch/include/internal/undef_atomics_int128.h CACHE INTERNAL "" FORCE)
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/CUDA" "ok=True
includepath=\"${CUDA_INCLUDE_PATH}\"
")


else()
file(WRITE "${PROJECT_BINARY_DIR}/data/build_info/CUDA" "ok=False
")
endif()
