/**
 *  \file IMP/random.h    \brief Random number generators used by IMP.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RANDOM_H
#define IMPKERNEL_RANDOM_H

#include <IMP/kernel_config.h>
#include <boost/random/mersenne_twister.hpp>

IMPKERNEL_BEGIN_NAMESPACE

#ifndef SWIG // SWIG is defined explicitly in pyext/IMP_kernel.random.i
typedef ::boost::mt19937_64 RandomNumberGenerator;

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
    @param p_random_array  preallocated array
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
get_random_doubles_normal(double* p_random_array, unsigned int n,
                          double mean=0.0, double stddev=1.0);

/** fill a pre-allocated array of n float numbers with
    random normally distributed values with specified mean and standard deviation
    @param p_random_array  preallocated array
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
get_random_floats_normal(float* p_random_array, unsigned int n,
                          float mean=0.0, float stddev=1.0);

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_RANDOM_H */
