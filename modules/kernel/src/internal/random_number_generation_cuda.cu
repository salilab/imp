/**
 *  \file IMP/kernel/src/internal/random_number_generation_cuda.cu
 *  \brief GPU or CPU pooled random number generation
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/internal/random_number_generation_cuda.h>


#ifdef __NVCC__

/* Using updated (v2) interfaces to cublas and cusparse */
#include <cuda_runtime.h>
#include <curand.h>
#include <IMP/internal/cuda_helper_functions.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>


static curandGenerator_t prngGPU; // GPU random number generator

void
IMPcuda::kernel::internal::get_random_numbers_normal_cuda
(double* p_random_array, unsigned int n,
 double mean, double stddev)
{
  double *d_Rand;
  IMP_checkCudaErrors(cudaMalloc((void **)&d_Rand, n * sizeof(double)));
  IMP_checkCudaErrors(curandGenerateNormalDouble(prngGPU, d_Rand, n, mean, stddev));
  IMP_checkCudaErrors(cudaMemcpy(p_random_array, d_Rand,
                                 n * sizeof(double), cudaMemcpyDeviceToHost));
  IMP_checkCudaErrors(cudaFree(d_Rand));
  IMP_checkCudaErrors(cudaDeviceSynchronize()); // DEBUG
}

void
IMPcuda::kernel::internal::get_random_numbers_normal_cuda
(float* p_random_array, unsigned int n,
 float mean, float stddev)
{
  float *d_Rand;
  IMP_checkCudaErrors(cudaMalloc((void **)&d_Rand, n * sizeof(float)));
  IMP_checkCudaErrors(curandGenerateNormal(prngGPU, d_Rand, n, mean, stddev));
  IMP_checkCudaErrors(cudaMemcpy(p_random_array, d_Rand,
                                 n * sizeof(float), cudaMemcpyDeviceToHost));
  IMP_checkCudaErrors(cudaFree(d_Rand));
}

void
IMPcuda::kernel::internal::get_random_numbers_uniform_cuda
(float* p_random_array, unsigned int n)
{
  float *d_Rand;
  IMP_checkCudaErrors(cudaMalloc((void **)&d_Rand, n * sizeof(float)));
  IMP_checkCudaErrors(curandGenerateUniform(prngGPU, d_Rand, n));
  IMP_checkCudaErrors(cudaMemcpy(p_random_array, d_Rand,
                                 n * sizeof(float), cudaMemcpyDeviceToHost));
  IMP_checkCudaErrors(cudaFree(d_Rand));
}

void
IMPcuda::kernel::internal::get_random_numbers_uniform_cuda
(double* p_random_array, unsigned int n)
{
  double *d_Rand;
  IMP_checkCudaErrors(cudaMalloc((void **)&d_Rand, n * sizeof(double)));
  IMP_checkCudaErrors(curandGenerateUniformDouble(prngGPU, d_Rand, n));
  IMP_checkCudaErrors(cudaMemcpy
                      (p_random_array, d_Rand,
                       n * sizeof(double), cudaMemcpyDeviceToHost));
  IMP_checkCudaErrors(cudaFree(d_Rand));
}

bool IMPcuda::kernel::internal::init_gpu_rng_once
(unsigned long long seed)
{
  static bool initialized(false); // is prngGPU initialized
  if(!initialized)
    {
      IMP_checkCudaErrors(curandCreateGenerator
                          (&prngGPU, CURAND_RNG_PSEUDO_MTGP32));
      IMP_checkCudaErrors(curandSetPseudoRandomGeneratorSeed(prngGPU, seed));
      initialized=true;
      return true; // success
    }
  return false; // was already initialized
}

void
IMPcuda::kernel::internal::destroy_gpu_rng
()
{
  IMP_checkCudaErrors(curandDestroyGenerator(prngGPU));
}

#endif
