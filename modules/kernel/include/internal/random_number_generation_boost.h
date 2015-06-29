/**
 *  \file IMP/kernel/internal/random_number_generation_boost.h
 *  \brief GPU or CPU pooled random number generation
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_RANDOM_NUMBER_GENERATION_BOOST_H
#define IMPKERNEL_INTERNAL_RANDOM_NUMBER_GENERATION_BOOST_H

#include <IMP/kernel_config.h>
#include <IMP/random.h>
#include <boost/random/normal_distribution.hpp>
#include <boost/random/uniform_real.hpp>
#include <iostream>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

    /** fill a pre-allocated array of n double numbers with random normally distributed values
        with specified mean and standard deviation

        @param p_random_array  preallocated array
        @param n  size of array
        @param mean  mean of normal distribution
        @param stddev  standard deviation
    */
template<typename RealType>
void get_random_numbers_normal_boost(RealType* p_random_array, unsigned int n,
                                  RealType mean, RealType stddev)
{
  typedef boost::variate_generator<RandomNumberGenerator &,
                                     boost::normal_distribution<RealType> > RNG;
  boost::normal_distribution<RealType> nd(mean, stddev);
  RNG sampler(IMP::random_number_generator, nd);
  for(unsigned int i=0; i<n; i++)
    {
      *(p_random_array++) = sampler();
      //      std::cout << "Rand #" << i << " " << *(p_random_array-1) << std::endl;
    }
}


/** fill a pre-allocated array of n float numbers with random
    uniformly distributed values within the [0..1) range

    @param p_random_array  preallocated array
    @param n  size of array
*/
template<typename RealType>
void get_random_numbers_uniform_boost(RealType* p_random_array,
                                          unsigned int n)
{
  boost::uniform_real<RealType> ud(0.0, 1.0);
  for(unsigned int i=0; i<n; i++)
    {
      *(p_random_array++) = ud(IMP::random_number_generator);
    }
}

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_RANDOM_NUMBER_GENERATION_BOOST_H */
