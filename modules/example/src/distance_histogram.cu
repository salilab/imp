////////////////////////////////////////////////////////////////////////////
//
// Copyright 1993-2012 NVIDIA Corporation.  All rights reserved.
//
// Please refer to the NVIDIA end user license agreement (EULA) associated
// with this source code for terms and conditions that govern your use of
// this software. Any use, reproduction, disclosure, or distribution of
// this software and related documentation outside the terms of the EULA
// is strictly prohibited.
//
////////////////////////////////////////////////////////////////////////////

/* Example of integrating CUDA functions into an existing
 * application / framework.
 * Host part of the device code.
 * Compiled with Cuda compiler.
 */

// System includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// CUDA runtime
#include <cuda_runtime.h>

// helper functions and utilities to work with CUDA
#include <IMP/scratch/internal/helper_cuda.h>
#include <IMP/scratch/internal/helper_functions.h>

#define SQ(x) (x)*(x)


__constant__ int DN; // number of atoms
__constant__ float DDRH; // histogram resolution

__global__ void gpu_histogram_kernel(float3 *in, float *nhis) {

  int iBlockBegin = (DN/gridDim.x)*blockIdx.x;
  int iBlockEnd = min((DN/gridDim.x)*(blockIdx.x+1),DN);
  int jBlockBegin = (DN/gridDim.y)*blockIdx.y;
  int jBlockEnd = min((DN/gridDim.y)*(blockIdx.y+1),DN);
  for (int i=iBlockBegin+threadIdx.x; i<iBlockEnd; i+=blockDim.x) {
    float3 a = in[i];
    for (int j=jBlockBegin+threadIdx.y; j<jBlockEnd; j+=blockDim.y) {
      if (i<j) {
        // Process (i,j) atom pair
        float3 b = in[j];
        float rij = sqrt(SQ(a.x - b.x) + SQ(a.y - b.y) + SQ(a.z - b.z));
        int ih = rij/DDRH;
        // nhis[ih] += 1.0; /* Entry to the histogram */
        atomicAdd(&nhis[ih],1.0);
      } // end if i<j
    } // end for j
  } // end for i
}


extern "C" void
get_distance_histogram_gpu(int argc, char ** argv, int len, float3 *coords, int hist_size, float *nhis)
{
  // use command-line specified CUDA device, otherwise use device with highest Gflops/s
  findCudaDevice(1, (const char **)argv);

  float delta = 0.5;
  cudaMemcpyToSymbol(DN, &len, sizeof(int), 0, cudaMemcpyHostToDevice);
  cudaMemcpyToSymbol(DDRH, &delta, sizeof(float), 0, cudaMemcpyHostToDevice);


  // allocate device memory
  float3 *dev_data; // atomic positions
  checkCudaErrors(cudaMalloc((void **) &dev_data, sizeof(float3) * len));
  float *dev_nhis; // histogram
  checkCudaErrors(cudaMalloc((void **) &dev_nhis, sizeof(float) * hist_size));

  // copy host memory to device
  checkCudaErrors(cudaMemcpy(dev_data, coords, sizeof(float3) * len,
                             cudaMemcpyHostToDevice));

  // initialize histogram with zeros
  cudaMemset(dev_nhis, 0.0, hist_size*sizeof(float));

  dim3 numBlocks(8,8,1);
  dim3 threads_per_block(16,16,1);
  gpu_histogram_kernel<<<numBlocks,threads_per_block>>>(dev_data, dev_nhis);

  // copy results from device to host
  checkCudaErrors(cudaMemcpy(nhis, dev_nhis, sizeof(float) * hist_size,
                             cudaMemcpyDeviceToHost));

  checkCudaErrors(cudaFree(dev_data));
  checkCudaErrors(cudaFree(dev_nhis));

}
