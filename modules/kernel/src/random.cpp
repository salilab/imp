/**
 *  \file random.cpp  \brief Random number generators used by IMP.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/random.h"
#include "IMP/internal/base_static.h"
#ifdef CUDA_LIB
#include "IMP/internal/random_number_generation_cuda.h"
#else
#include "IMP/internal/random_number_generation_boost.h"
#endif

IMPKERNEL_BEGIN_NAMESPACE
boost::uint64_t get_random_seed()
{
  return static_cast<boost::uint64_t>(internal::random_seed);
}


void get_random_doubles_normal
(double* p_random_array, unsigned int n,
 double mean, double stddev)
{
  #ifdef CUDA_LIB
  IMPcuda::kernel::internal::init_gpu_rng_once(get_random_seed());
  IMPcuda::kernel::internal::get_random_numbers_normal_cuda
    (p_random_array, n, mean, stddev);
  #else
  internal::get_random_numbers_normal_boost(p_random_array, n, mean, stddev);
  #endif
}

void get_random_floats_normal
(float* p_random_array, unsigned int n,
 float mean, float stddev)
{
  #ifdef CUDA_LIB
  IMPcuda::kernel::internal::init_gpu_rng_once(get_random_seed());
  IMPcuda::kernel::internal::get_random_numbers_normal_cuda
    (p_random_array, n, mean, stddev);
  #else
  internal::get_random_numbers_normal_boost(p_random_array, n, mean, stddev);
  #endif
}

IMPKERNEL_END_NAMESPACE
