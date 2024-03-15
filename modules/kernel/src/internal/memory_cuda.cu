/**
 *  \file IMP/kernel/src/internal/memory_cuda.cu
 *  \brief CUDA memory allocation functions
 *
 *  Copyright 2007-2024 IMP Inventors. All rights reserved.
 *
 */


#ifdef __NVCC__
#include <cuda_runtime.h>
#include <IMP/internal/memory_cuda.h>
#include <IMP/internal/cuda_helper_functions.h>

void *IMPcuda::kernel::internal::allocate_unified_cuda(std::size_t n) {
  void *p;
  IMP_checkCudaErrors(cudaMallocManaged(&p, n));
  return p;
}

void IMPcuda::kernel::internal::deallocate_unified_cuda(void *p) {
  if (p) {
    IMP_checkCudaErrors(cudaFree(p));
  }
}

#endif
