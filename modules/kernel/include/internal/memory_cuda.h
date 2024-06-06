/**
 *  \file IMP/kernel/internal/memory_cuda.h
 *  \brief CUDA memory allocation functions
 *
 *  Copyright 2007-2024 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_MEMORY_CUDA_H
#define IMPKERNEL_INTERNAL_MEMORY_CUDA_H

namespace IMPcuda {
  namespace kernel {
    namespace internal {

      //! Use CUDA API to allocate unified memory
      void *allocate_unified_cuda(std::size_t n);

      //! Use CUDA API to free unified memory
      void deallocate_unified_cuda(void *p);
    }
  }
}

#endif /* IMPKERNEL_INTERNAL_MEMORY_CUDA_H */
