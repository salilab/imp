/**
 *  \file IMP/random_utils.h    \brief Random number utility functions used by IMP.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RANDOM_UTILS_H
#define IMPKERNEL_RANDOM_UTILS_H

#include <IMP/kernel_config.h>
#include <IMP/Vector.h>
#include <IMP/random.h>
#ifdef IMP_KERNEL_CUDA_LIB
//  #warning "random_utils - kernel CUDA_LIB!"
#include "IMP/internal/random_number_generation_cuda.h"
#else
//  #warning "random_utils - kernel CUDA_BOOST!"
#include "IMP/internal/random_number_generation_boost.h"
#endif

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>

// #include <ctime> // DEBUG
//#include <sys/time.h> // DEBUG

IMPKERNEL_BEGIN_NAMESPACE

//! Fill the double array with random normally distributed values.
/** The pre-allocated array is filled with n double numbers with
    random normally distributed values with specified mean and
    standard deviation.

    @param v  vector array that will be resized to n
    @param n  size of array
    @param mean  mean of normal distribution
    @param stddev  standard deviation

    @note Implementation relies on random_number_generator (a boost
          random number generator), or on the CUDA random number
          generator if kernel is built with IMP_KERNEL_CUDA_LIB cmake
          flag. Either is initially seeded with get_random_seed().
*/
template<typename RealType>
void get_random_numbers_normal
(Vector<RealType>& v, unsigned int n,
 RealType mean=0.0, RealType stddev=1.0)
{
  if(n==0) return;
  if(n>v.size())
    v.resize(n);
  //  struct timeval  start_time; // DEBUG
  //struct timeval t_time; // DEBUG
  //gettimeofday(&start_time, 0); // DEBUG
#ifdef IMP_KERNEL_CUDA_LIB
  IMPcuda::kernel::internal::init_gpu_rng_once(get_random_seed());
  IMPcuda::kernel::internal::get_random_numbers_normal_cuda
    (&v[0], n, mean, stddev);
#else
  internal::get_random_numbers_normal_boost(&v[0], n, mean, stddev);
#endif
  //  gettimeofday(&t_time, 0); // DEBUG
  // double time_diff_sec= (double)( (t_time.tv_sec - start_time.tv_sec)
  //                                + 0.000001 * (t_time.tv_usec - start_time.tv_usec) ); // DEBUG
  // std::cout << "get_random_numbers_uniform_cuda(" << n
  //          << ") " << time_diff_sec << " seconds" << std::endl; // DEBUG

}

//! Fill the float array with random uniformly distributed values.
/** Fill a pre-allocated array of n float numbers with random uniformly
    distributed values in the [0..1) range.

    @param v  vector array that will be resized to n
    @param n  size of array

    @note Implementation relies on random_number_generator (a boost
          random number generator), or on the CUDA random number generator
          if kernel is built with CUDA flag. Either is initially seeded with
          get_random_seed().
    */
template<typename RealType>
void get_random_numbers_uniform
(Vector<RealType>& v, unsigned int n)
{
  if(n==0) return;
  if(n>v.size())
    v.resize(n);
#ifdef IMP_KERNEL_CUDA_LIB
  IMPcuda::kernel::internal::init_gpu_rng_once(get_random_seed());
  IMPcuda::kernel::internal::get_random_numbers_uniform_cuda (&v[0], n);
#else
  internal::get_random_numbers_uniform_boost(&v[0], n);
#endif
}

//! Return a uniformly distributed float number in range [0..1)
/** @note the random number is retrieved from a cache of random
    numbers generated using GPU if compiled with CUDA, or from boost
    without a cache otherwise.
 */
float get_random_float_uniform();

//! Return a uniformly distributed float number in range [min..max)
/** @note the random number is retrieved from a cache of random
    numbers generated using GPU if compiled with CUDA, or from boost
    without a cache otherwise.
 */
float get_random_float_uniform(float min, float max);

//! Return a uniformly distributed double number in range [0..1)
/** @note the random number is retrieved from a cache of random
    numbers generated using GPU if compiled with CUDA, or from boost
    without a cache otherwise.
 */
double get_random_double_uniform();

//! Return a uniformly distributed double number in range [min..max)
/** @note the random number is retrieved from a cache of random
    numbers generated using GPU if compiled with CUDA, or from boost
    without a cache otherwise.
 */
double get_random_double_uniform(double min, double max);

/************ implementation of inline functions *******/

inline float
get_random_float_uniform()
{
  // use cache only with cuda
#ifdef IMP_KERNEL_CUDA_LIB
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
#ifdef IMP_KERNEL_CUDA_LIB
  return get_random_float_uniform()*(max-min)+min;
#else
  ::boost::uniform_real<float> rand(min, max);
  return rand(random_number_generator);
#endif
}


inline double
get_random_double_uniform()
{
#ifdef IMP_KERNEL_CUDA_LIB
  const static unsigned int cache_n=2000000;
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
#ifdef IMP_KERNEL_CUDA_LIB
  return get_random_double_uniform()*(max-min)+min;
#else
  ::boost::uniform_real<double> rand(min, max);
  return rand(random_number_generator);
#endif
}



IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_RANDOM_UTILS_H */
