/**
 *  \file IMP/random.h    \brief Random number generators used by IMP.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RANDOM_H
#define IMPKERNEL_RANDOM_H

#include <IMP/kernel_config.h>
#include <IMP/Vector.h>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>

IMPKERNEL_BEGIN_NAMESPACE

#ifndef SWIG // the RNG is defined explicitly in pyext/IMP_kernel.random.i
typedef ::boost::mt19937 RandomNumberGenerator;

//! A shared non-GPU random number generator
/** The random number generator is seeded based on command line specified flag.
   Otherwise, the default seed is retrieved from among either
   boost::random_device, `/dev/urandom`, or the system clock, based on which
   method is available in this priority order.

This generator can be used by the
[Boost.Random](http://www.boost.org/doc/libs/1_39_0/libs/random/index.html)
distributions.
 */
extern IMPKERNELEXPORT RandomNumberGenerator random_number_generator;
#endif

//! Return the initial random seed.
IMPKERNELEXPORT boost::uint64_t get_random_seed();

/** fill a pre-allocated array of n double numbers with
    random normally distributed values with specified mean and standard deviation
    @param v  vector array that will be resized to n
    @param n  size of array
    @param mean  mean of normal distribution
    @param stddev  standard deviation

    @note Implelmentation relies on random_number_generator (a boost
          random number generator), or on the CUDA random number generator
          if kernel is built with CUDA flag. Either is initially seeded with
          get_random_seed().
    @note To the advanced developer: this function defyies standard IMP
          conventions, and uses a C double array rather than the
          std::vector class. The is more efficient for copying very large
          arrays of random numbers from eg GPU - hence this should not be
          changed to std::vector without much thought.
    */
IMPKERNELEXPORT void
get_random_doubles_normal(Vector<double>& v, unsigned int n,
                          double mean=0.0, double stddev=1.0);

/** fill a pre-allocated array of n float numbers with
    random normally distributed values with specified mean and standard deviation
    @param v  vector array that will be resized to n
    @param n  size of array
    @param mean  mean of normal distribution
    @param stddev  standard deviation

    @note Implelmentation relies on random_number_generator (a boost
          random number generator), or on the CUDA random number generator
          if kernel is built with CUDA flag. Either is initially seeded with
          get_random_seed().
    @note To the advanced developer: this function defyies standard IMP
          conventions, and uses a C float array rather than the
          std::vector class. The is more efficient for copying very large
          arrays of random numbers from eg GPU - hence this should not be
          changed to std::vector without much thought.
    */
IMPKERNELEXPORT void
get_random_floats_normal(Vector<float>& v, unsigned int n,
                          float mean=0.0, float stddev=1.0);


/** fill a pre-allocated array of n float numbers with
    random uniformly distributed values within [0..1)
    @param v  vector array that will be resized to n
    @param n  size of array

    @note Implelmentation relies on random_number_generator (a boost
          random number generator), or on the CUDA random number generator
          if kernel is built with CUDA flag. Either is initially seeded with
          get_random_seed().
    @note To the advanced developer: this function defyies standard IMP
          conventions, and uses a C float array rather than the
          std::vector class. The is more efficient for copying very large
          arrays of random numbers from eg GPU - hence this should not be
          changed to std::vector without much thought.
    */
IMPKERNELEXPORT void
get_random_floats_uniform(Vector<float>& v, unsigned int n);

/** fill a pre-allocated array of n double numbers with
    random uniformly distributed values within [0..1)
    @param v  vector array that will be resized to n
    @param n  size of array

    @note Implelmentation relies on random_number_generator (a boost
          random number generator), or on the CUDA random number generator
          if kernel is built with CUDA flag. Either is initially seeded with
          get_random_seed().
    @note To the advanced developer: this function defyies standard IMP
          conventions, and uses a C double array rather than the
          std::vector class. The is more efficient for copying very large
          arrays of random numbers from eg GPU - hence this should not be
          changed to std::vector without much thought.
    */
IMPKERNELEXPORT void
get_random_doubles_uniform(Vector<double>& v, unsigned int n);

//! return a uniformly distributed floatOB number in range [0..1)
//!
//! @note the random number is retrieved from a cache of random
//! numbers generated using GPU if compiled with CUDA, or from boost
//! without a cache otherwise.
inline float
get_random_float_uniform();

//! return a uniformly distributed float number in range [min..max)
//!
//! @note the random number is retrieved from a cache of random
//! numbers generated using GPU if compiled with CUDA, or from boost
//! without a cache otherwise.
inline float
get_random_float_uniform(float min, float max);

//! return a uniformly distributed double number in range [0..1)
//!
//! @note the random number is retrieved from a cache of random
//! numbers generated using GPU if compiled with CUDA, or from boost
//! without a cache otherwise.
inline double
get_random_double_uniform();

//! return a uniformly distributed double number in range [min..max)
//!
//! @note the random number is retrieved from a cache of random
//! numbers generated using GPU if compiled with CUDA, or from boost
//! without a cache otherwise.
inline double
get_random_double_uniform(double min, double max);

/************ implementation of inline functions *******/

inline float
get_random_float_uniform()
{
#ifdef CUDA_LIB
  const static unsigned int cache_n=20000000;
  static IMP::Vector<float> cache;
  static unsigned int i=0;
  if(i>=cache.size()){
    get_random_floats_uniform(cache, cache_n);
    i=0;
  }
  return cache[i++];
#else
  static boost::random::uniform_real_distribution<float> rand(0.0, 1.0);
  return rand(random_number_generator);
#endif
}

inline float
get_random_float_uniform(float min, float max)
{
#ifdef CUDA_LIB
  return get_random_float_uniform()*(max-min)+min;
#else
  ::boost::random::uniform_real_distribution<float> rand(min, max);
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
    get_random_doubles_uniform(cache, cache_n);
    i=0;
  }
  return cache[i++];
#else
  static boost::random::uniform_real_distribution<double> rand(0.0, 1.0);
  return rand(random_number_generator);
#endif
}

inline double
get_random_double_uniform(double min, double max)
{
#ifdef CUDA_LIB
  return get_random_double_uniform()*(max-min)+min;
#else
  ::boost::random::uniform_real_distribution<double> rand(min, max);
  return rand(random_number_generator);
#endif
}



IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_RANDOM_H */
