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

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

    /** fill a pre-allocated array of n double numbers with random normally distributed values
        with specified mean and standard deviation

        @param p_random_array  preallocated array
        @param n  size of array
        @param mean  mean of normal distribution
        @param stddev  standard deviation

        @note Assumes RNG is preinitialized with init_gpu_rng
    */
    void get_random_numbers_normal_boost(double* p_random_array,
                                         unsigned int n,
                                         double mean=0.0,
                                         double stddev=1.0);

    /** fill a pre-allocated array of n float numbers with random normally distributed values
        with specified mean and standard deviation

        @param p_random_array  preallocated array
        @param n  size of array
        @param mean  mean of normal distribution
        @param stddev  standard deviation

        @note Assumes RNG is preinitialized with init_gpu_rng
    */
    void get_random_numbers_normal_boost(float* p_random_array,
                                         unsigned int n,
                                         float mean=0.0,
                                         float stddev=1.0);


IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_RANDOM_NUMBER_GENERATION_BOOST_H */
