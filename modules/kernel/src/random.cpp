/**
 *  \file random.cpp  \brief Random number generators used by IMP.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/random.h"
#include "IMP/internal/base_static.h"
#include <IMP/Vector.h>
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
(Vector<double>& v, unsigned int n,
 double mean, double stddev)
{
  if(n==0) return;
  if(n>v.size())
    v.resize(n);
  #ifdef CUDA_LIB
  IMPcuda::kernel::internal::init_gpu_rng_once(get_random_seed());
  IMPcuda::kernel::internal::get_random_numbers_normal_cuda
    (&v[0], n, mean, stddev);
  #else
  internal::get_random_numbers_normal_boost(&v[0], n, mean, stddev);
  #endif
}

void get_random_floats_normal
(Vector<float>& v, unsigned int n,
 float mean, float stddev)
{
  if(n==0) return;
  if(n>v.size())
    v.resize(n);
  #ifdef CUDA_LIB
  IMPcuda::kernel::internal::init_gpu_rng_once(get_random_seed());
  IMPcuda::kernel::internal::get_random_numbers_normal_cuda
    (&v[0], n, mean, stddev);
  #else
  internal::get_random_numbers_normal_boost(&v[0], n, mean, stddev);
  #endif
}

void
get_random_floats_uniform
(Vector<float>& v, unsigned int n)
{
  if(n==0) return;
  if(n>v.size())
    v.resize(n);
  #ifdef CUDA_LIB
  IMPcuda::kernel::internal::init_gpu_rng_once(get_random_seed());
  IMPcuda::kernel::internal::get_random_numbers_uniform_cuda (&v[0], n);
  #else
  internal::get_random_numbers_uniform_boost(&v[0], n);
  #endif
}

void
get_random_doubles_uniform
(Vector<double>& v, unsigned int n)
{
  if(n==0) return;
  if(n>v.size())
    v.resize(n);
  #ifdef CUDA_LIB
  IMPcuda::kernel::internal::init_gpu_rng_once(get_random_seed());
  IMPcuda::kernel::internal::get_random_numbers_uniform_cuda (&v[0], n);
  #else
  internal::get_random_numbers_uniform_boost(&v[0], n);
  #endif
}


IMPKERNEL_END_NAMESPACE
