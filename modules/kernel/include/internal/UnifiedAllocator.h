/**
 *  \file IMP/kernel/internal/UnifiedAllocator.h
 *  \brief Allocator that uses CUDA unified memory
 *
 *  Copyright 2007-2024 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_UNIFIED_ALLOCATOR_H
#define IMPKERNEL_INTERNAL_UNIFIED_ALLOCATOR_H

#include <IMP/internal/memory_cuda.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

//! Use CUDA unified memory to back STL containers
/** Can be used as the second argument to many containers
    (e.g. std::vector) to ensure that the memory they use is accessible
    from both the CPU and the GPU. */
template<typename T> class UnifiedAllocator {
public:
  typedef T value_type;
  typedef T* pointer;
  typedef std::size_t size_type;

  UnifiedAllocator() noexcept = default;

  T* allocate(std::size_t n, const void* = 0) {
    return (T*)IMPcuda::kernel::internal::allocate_unified_cuda(n * sizeof(T));
  }

  void deallocate(T *p, std::size_t) {
    IMPcuda::kernel::internal::deallocate_unified_cuda(p);
  }
};

template <class T, class U>
bool operator==(UnifiedAllocator<T> const &, UnifiedAllocator<U> const &) {
  return true;
}

template <class T, class U>
bool operator!=(UnifiedAllocator<T> const &, UnifiedAllocator<U> const &) {
  return false;
}

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_UNIFIED_ALLOCATOR_H */
