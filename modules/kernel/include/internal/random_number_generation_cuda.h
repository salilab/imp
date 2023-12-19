/**
 *  \file internal/random_number_generation_cuda.h
 *  \brief GPU pooled random number generation
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_RANDOM_NUMBER_GENERATION_CUDA_H
#define IMPKERNEL_INTERNAL_RANDOM_NUMBER_GENERATION_CUDA_H

//#include <IMP/kernel_config.h>

namespace IMPcuda {
  namespace kernel {
    namespace internal {

    /** fill a pre-allocated array of n double numbers with random normally distributed values
        with specified mean and standard deviation

        @param p_random_array  preallocated array
        @param n  size of array
        @param mean  mean of normal distribution
        @param stddev  standard deviation

        @note Assumes RNG is preinitialized with init_gpu_rng
    */
    void get_random_numbers_normal_cuda(double* p_random_array, unsigned int n,
                                        double mean=0.0, double stddev=1.0);

    /** fill a pre-allocated array of n float numbers with random normally distributed values
        with specified mean and standard deviation

        @param p_random_array  preallocated array
        @param n  size of array
        @param mean  mean of normal distribution
        @param stddev  standard deviation

        @note Assumes RNG is preinitialized with init_gpu_rng
    */
    void get_random_numbers_normal_cuda(float* p_random_array, unsigned int n,
                                        float mean=0.0, float stddev=1.0);


        /**
            fill a pre-allocated array of n float numbers with random uniformly distributed values
            within the [0..1) range

            @param p_random_array  preallocated array
            @param n  size of array

            @note Assumes RNG is preinitialized with init_gpu_rng
        */
    void get_random_numbers_uniform_cuda(float* p_random_array,
                                         unsigned int n);

        /**
            fill a pre-allocated array of n double numbers with random uniformly distributed values
            within the [0..1) range

            @param p_random_array  preallocated array
            @param n  size of array

            @note Assumes RNG is preinitialized with init_gpu_rng
        */
    void get_random_numbers_uniform_cuda(double* p_random_array,
                                         unsigned int n);

    /** Initialize the cuda rng, if not initialized already.
        Also, set the seed if necessary

     @param seed random seed
     @param seed_counter the time when the non-GPU seed was last changed
     @param last_seeded the time when the CUDA rng seed was last set

     @return true if successful, false if rng has already been initialized
    */
      bool init_gpu_rng_once(unsigned long long seed,
                             unsigned seed_counter, unsigned &last_seeded);

      /** destroys the cuda gpu object */
      void destroy_gpu_rng();


    };
  };
};
#endif /* IMPKERNEL_INTERNAL_RANDOM_NUMBER_GENERATION_CUDA_H */
