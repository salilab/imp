/**
 * \file cuda_helpers.cu
 * \brief GPU implementations of some SAXS operations
 *
 * Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/internal/cuda_helper_functions.h>
#include <IMP/saxs/internal/cuda_helpers.h>
#include <iostream>
#include <cmath>
#include <stdio.h>

namespace IMPcuda {
namespace saxs {
namespace internal {

const static size_t MAX_THREADS = 512;

template <class T>
__device__ T square(T t) {
  return t * t;
}

__device__ double sinc_pi(double x) {
  if (x < 1e-6) {
    return 1.;
  } else {
    return sin(x) / x;
  }
}

__global__
void make_profile(const double *r_dist, const float *q, const double *distances,
                  float *intensity, double modulation_function_parameter,
                  size_t r_size, size_t q_size)
{
  __shared__ float inten[MAX_THREADS];
  // one block per q; each thread handles part of r_dist
  size_t k = blockIdx.x;
  inten[threadIdx.x] = 0.0;
  // iterate over radial distribution
  for (unsigned int r = threadIdx.x; r < r_size; r += blockDim.x) {
    double x = distances[r] * q[k];
    x = sinc_pi(x);
    // multiply by the value from distribution
    inten[threadIdx.x] += r_dist[r] * x;
  }
  __syncthreads();
  // get total intensity in first thread
  if (threadIdx.x == 0) {
    float total = 0.;
    for (size_t i = 0; i < blockDim.x; ++i) {
      total += inten[i];
    }
    // this correction is required since we approximate the form factor
    // as f(q) = f(0) * exp(-b*q^2)
    intensity[k] = total * std::exp(-modulation_function_parameter
                                    * square(q[k]));
  }
}

void squared_distribution_2_profile_cuda(
           const double *r_dist, const float *q,
           const double *distances, float *intensity,
           double modulation_function_parameter, size_t r_size, size_t q_size)
{
  double *d_r_dist, *d_distances;
  float *d_q, *d_intensity;

  IMP_checkCudaErrors(cudaMalloc(&d_r_dist, r_size * sizeof(double)));
  IMP_checkCudaErrors(cudaMalloc(&d_distances, r_size * sizeof(double)));
  IMP_checkCudaErrors(cudaMalloc(&d_q, q_size * sizeof(float)));
  IMP_checkCudaErrors(cudaMalloc(&d_intensity, q_size * sizeof(float)));

  // copy inputs to device
  IMP_checkCudaErrors(cudaMemcpy(d_r_dist, r_dist, r_size * sizeof(double),
                                 cudaMemcpyHostToDevice));
  IMP_checkCudaErrors(cudaMemcpy(d_distances, distances,
                                 r_size * sizeof(double),
                                 cudaMemcpyHostToDevice));
  IMP_checkCudaErrors(cudaMemcpy(d_q, q, q_size * sizeof(float),
                                 cudaMemcpyHostToDevice));

  size_t n_threads = std::min(MAX_THREADS, r_size);
  make_profile<<<q_size, n_threads>>>(d_r_dist, d_q, d_distances, d_intensity,
                                      modulation_function_parameter, r_size,
                                      q_size);
  IMP_checkCudaErrors(cudaDeviceSynchronize());

  // copy outputs back to host
  IMP_checkCudaErrors(cudaMemcpy(intensity, d_intensity, q_size * sizeof(float),
                                 cudaMemcpyDeviceToHost));

  IMP_checkCudaErrors(cudaFree(d_r_dist));
  IMP_checkCudaErrors(cudaFree(d_distances));
  IMP_checkCudaErrors(cudaFree(d_q));
  IMP_checkCudaErrors(cudaFree(d_intensity));
}

} } }
