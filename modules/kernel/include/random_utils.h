/**
 *  \file IMP/random_utils.h    \brief Random number utility functions used by IMP.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RANDOM_UTILS_H
#define IMPKERNEL_RANDOM_UTILS_H

#include <IMP/kernel_config.h>
#include <IMP/Vector.h>
#include <IMP/random.h>
#ifdef CUDA_LIB
#include "IMP/internal/random_number_generation_cuda.h"
#else
#include "IMP/internal/random_number_generation_boost.h"
#endif

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>

IMPKERNEL_BEGIN_NAMESPACE

/** fill a pre-allocated array of n normally distributed real numbers with
    specified mean and standard deviation

    @param v  vector array that will be resized to n
    @param n  size of array
    @param mean  mean of normal distribution
    @param stddev  standard deviation

    @note Implelmentation relies on random_number_generator (a boost
          random number generator), or on the CUDA random number
          generator if kernel is built with CUDA_LIB cmake
          flag. Either is initially seeded with get_random_seed().
    @note To the advanced developer: this function defyies standard IMP
          conventions, and uses a C double array rather than the
          std::vector class. The is more efficient for copying very large
          arrays of random numbers from eg GPU - hence this should not be
          changed to std::vector without much thought.
*/
template<typename RealType>
void get_random_numbers_normal
//(Vector<RealType>& v, unsigned int n,
(std::vector<RealType>& v, unsigned int n,
 RealType mean=0.0, RealType stddev=1.0)
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

/** fill a pre-allocated array of n real numbers with
    random uniformly distributed values within [0..1)
    @param v  vector array that will be resized to n
    @param n  size of array

    @note Implelmentation relies on random_number_generator (a boost
          random number generator), or on the CUDA random number generator
          if kernel is built with CUDA flag. Either is initially seeded with
          get_random_seed().
    @note To the advanced developer: this function defyies standard IMP
          conventions, and uses a C real array rather than the
          std::vector class. The is more efficient for copying very large
          arrays of random numbers from eg GPU - hence this should not be
          changed to std::vector without much thought.
    */
template<typename RealType>
void get_random_numbers_uniform
(std::vector<RealType>& v, unsigned int n)
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

//! return a uniformly distributed floatOAOB number in range [0..1)
//!
//! @note the random number is retrieved from a cache of random
//! numbers generated using GPU if compiled with CUDA, or from boost
//! without a cache otherwise.
IMPKERNELEXPORT float
get_random_float_uniform();

//! return a uniformly distributed float number in range [min..max)
//!
//! @note the random number is retrieved from a cache of random
//! numbers generated using GPU if compiled with CUDA, or from boost
//! without a cache otherwise.
IMPKERNELEXPORT float
get_random_float_uniform(float min, float max);

//! return a uniformly distributed double number in range [0..1)
//!
//! @note the random number is retrieved from a cache of random
//! numbers generated using GPU if compiled with CUDA, or from boost
//! without a cache otherwise.
IMPKERNELEXPORT double
get_random_double_uniform();

//! return a uniformly distributed double number in range [min..max)
//!
//! @note the random number is retrieved from a cache of random
//! numbers generated using GPU if compiled with CUDA, or from boost
//! without a cache otherwise.
IMPKERNELEXPORT double
get_random_double_uniform(double min, double max);

/************ implementation of inline functions *******/

inline float
get_random_float_uniform()
{
  // use cache only with cuda
#ifdef CUDA_LIB
  const static unsigned int cache_n=20000000;
  static IMP::Vector<float> cache;
  static unsigned int i=0;
  if(i>=cache.size()){
    get_random_numbers_uniform(cache, cache_n);
    i=0;
  }
  return cache[i++];
#else
  static boost::uniform_real<float> rand(0.0, 1.0);
  return rand(random_number_generator);
#endif
}

inline float
get_random_float_uniform(float min, float max)
{
  // use cache only with cuda
#ifdef CUDA_LIB
  return get_random_float_uniform()*(max-min)+min;
#else
  ::boost::uniform_real<float> rand(min, max);
  return rand(random_number_generator);
#endif
}


inline double
get_random_double_uniform()
{
#ifdef CUDA_LIB
  const static unsigned int cache_n=20000000;
  static IMP::Vector<double> cache;
  static unsigned int i=0;
  if(i>=cache.size()){
    get_random_numbers_uniform(cache, cache_n);
    i=0;
  }
  return cache[i++];
#else
  static boost::uniform_real<double> rand(0.0, 1.0);
  return rand(random_number_generator);
#endif
}

inline double
get_random_double_uniform(double min, double max)
{
#ifdef CUDA_LIB
  return get_random_double_uniform()*(max-min)+min;
#else
  ::boost::uniform_real<double> rand(min, max);
  return rand(random_number_generator);
#endif
}



IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_RANDOM_UTILS_H */
