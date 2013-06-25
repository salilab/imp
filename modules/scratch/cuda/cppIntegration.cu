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

///////////////////////////////////////////////////////////////////////////////
//! Demonstration that int2 data can be used in the cpp code
//! @param g_odata  memory to process (in and out)
///////////////////////////////////////////////////////////////////////////////
__global__ void
kernel(int len, double3 *in, double *out)
{
    // write data to global memor
  const unsigned int i = blockIdx.x;
  //const unsigned int j = threadIdx.y;
  double3 a = in[i];
  for (unsigned int j = 0; j < len; ++j) {
    //const unsigned int j = threadIdx.x;
    double3 b = in[j];
    out[i * len + j] = sqrt(SQ(a.x - b.x) + SQ(a.y - b.y) + SQ(a.z - b.z));
  }
}

extern "C" void
get_distances_gpu(int argc, char ** argv, int len, double3 *coords, double *out)
{
    // use command-line specified CUDA device, otherwise use device with highest Gflops/s
  findCudaDevice(1, (const char **)argv);

  // allocate device memory
    double3 *d_data;
    checkCudaErrors(cudaMalloc((void **) &d_data, sizeof(double3) * len));
    double *out_data;
    checkCudaErrors(cudaMalloc((void **) &out_data, sizeof(double) * len * len));

    // copy host memory to device
    checkCudaErrors(cudaMemcpy(d_data, coords, sizeof(double3) * len,
                               cudaMemcpyHostToDevice));

    checkCudaErrors(cudaMemcpy(out_data, out, sizeof(double) * len * len,
      cudaMemcpyHostToDevice));


    dim3 threads(1, 1, 1);
    // setup execution parameters
    dim3 grid(len, 1, 1);

    // execute the kernel
    kernel<<< grid, threads >>>(len, d_data, out_data);

    // copy results from device to host
    checkCudaErrors(cudaMemcpy(out, out_data, sizeof(double) * len * len,
                               cudaMemcpyDeviceToHost));

    checkCudaErrors(cudaFree(d_data));
    checkCudaErrors(cudaFree(out_data));

}
