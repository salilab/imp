find_package(CUDA)

cuda_compile(CUDA_TARGET cppIntegration.cu)
add_custom_target(IMP.scratch.cuda DEPENDS ${CUDA_TARGET})
set(IMP_SCRATCH_LIBRARY_EXTRA_DEPENDENCIES IMP.scratch.cuda CACHE INTERNAL "")
set(IMP_SCRATCH_LIBRARY_EXTRA_SOURCES ${CUDA_TARGET} CACHE INTERNAL "")

CUDA_ADD_EXECUTABLE(vectorAdd vectorAdd.cu)
