/**
 *  \file IMP/kernel/random_number_generation.h
 *  \brief GPU or CPU pooled random number generation
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_INTERNAL_CUDA_HELPER_FUNCTIONS_H
#define IMPKERNEL_INTERNAL_CUDA_HELPER_FUNCTIONS_H
//#include <IMP/kernel_config.h>

#include <stdlib.h>
#include <stdio.h>

namespace IMPcuda {
  namespace kernel {
    namespace internal {

      // CUDA Runtime error messages
#ifdef __DRIVER_TYPES_H__
      static const char *_cudaGetErrorEnum(cudaError_t error)
      {
        switch (error)
          {
          case cudaSuccess:
            return "cudaSuccess";

          case cudaErrorMissingConfiguration:
            return "cudaErrorMissingConfiguration";

          case cudaErrorMemoryAllocation:
            return "cudaErrorMemoryAllocation";

          case cudaErrorInitializationError:
            return "cudaErrorInitializationError";

          case cudaErrorLaunchFailure:
            return "cudaErrorLaunchFailure";

          case cudaErrorPriorLaunchFailure:
            return "cudaErrorPriorLaunchFailure";

          case cudaErrorLaunchTimeout:
            return "cudaErrorLaunchTimeout";

          case cudaErrorLaunchOutOfResources:
            return "cudaErrorLaunchOutOfResources";

          case cudaErrorInvalidDeviceFunction:
            return "cudaErrorInvalidDeviceFunction";

          case cudaErrorInvalidConfiguration:
            return "cudaErrorInvalidConfiguration";

          case cudaErrorInvalidDevice:
            return "cudaErrorInvalidDevice";

          case cudaErrorInvalidValue:
            return "cudaErrorInvalidValue";

          case cudaErrorInvalidPitchValue:
            return "cudaErrorInvalidPitchValue";

          case cudaErrorInvalidSymbol:
            return "cudaErrorInvalidSymbol";

          case cudaErrorMapBufferObjectFailed:
            return "cudaErrorMapBufferObjectFailed";

          case cudaErrorUnmapBufferObjectFailed:
            return "cudaErrorUnmapBufferObjectFailed";

          case cudaErrorInvalidHostPointer:
            return "cudaErrorInvalidHostPointer";

          case cudaErrorInvalidDevicePointer:
            return "cudaErrorInvalidDevicePointer";

          case cudaErrorInvalidTexture:
            return "cudaErrorInvalidTexture";

          case cudaErrorInvalidTextureBinding:
            return "cudaErrorInvalidTextureBinding";

          case cudaErrorInvalidChannelDescriptor:
            return "cudaErrorInvalidChannelDescriptor";

          case cudaErrorInvalidMemcpyDirection:
            return "cudaErrorInvalidMemcpyDirection";

          case cudaErrorAddressOfConstant:
            return "cudaErrorAddressOfConstant";

          case cudaErrorTextureFetchFailed:
            return "cudaErrorTextureFetchFailed";

          case cudaErrorTextureNotBound:
            return "cudaErrorTextureNotBound";

          case cudaErrorSynchronizationError:
            return "cudaErrorSynchronizationError";

          case cudaErrorInvalidFilterSetting:
            return "cudaErrorInvalidFilterSetting";

          case cudaErrorInvalidNormSetting:
            return "cudaErrorInvalidNormSetting";

          case cudaErrorMixedDeviceExecution:
            return "cudaErrorMixedDeviceExecution";

          case cudaErrorCudartUnloading:
            return "cudaErrorCudartUnloading";

          case cudaErrorUnknown:
            return "cudaErrorUnknown";

          case cudaErrorNotYetImplemented:
            return "cudaErrorNotYetImplemented";

          case cudaErrorMemoryValueTooLarge:
            return "cudaErrorMemoryValueTooLarge";

          case cudaErrorInvalidResourceHandle:
            return "cudaErrorInvalidResourceHandle";

          case cudaErrorNotReady:
            return "cudaErrorNotReady";

          case cudaErrorInsufficientDriver:
            return "cudaErrorInsufficientDriver";

          case cudaErrorSetOnActiveProcess:
            return "cudaErrorSetOnActiveProcess";

          case cudaErrorInvalidSurface:
            return "cudaErrorInvalidSurface";

          case cudaErrorNoDevice:
            return "cudaErrorNoDevice";

          case cudaErrorECCUncorrectable:
            return "cudaErrorECCUncorrectable";

          case cudaErrorSharedObjectSymbolNotFound:
            return "cudaErrorSharedObjectSymbolNotFound";

          case cudaErrorSharedObjectInitFailed:
            return "cudaErrorSharedObjectInitFailed";

          case cudaErrorUnsupportedLimit:
            return "cudaErrorUnsupportedLimit";

          case cudaErrorDuplicateVariableName:
            return "cudaErrorDuplicateVariableName";

          case cudaErrorDuplicateTextureName:
            return "cudaErrorDuplicateTextureName";

          case cudaErrorDuplicateSurfaceName:
            return "cudaErrorDuplicateSurfaceName";

          case cudaErrorDevicesUnavailable:
            return "cudaErrorDevicesUnavailable";

          case cudaErrorInvalidKernelImage:
            return "cudaErrorInvalidKernelImage";

          case cudaErrorNoKernelImageForDevice:
            return "cudaErrorNoKernelImageForDevice";

          case cudaErrorIncompatibleDriverContext:
            return "cudaErrorIncompatibleDriverContext";

          case cudaErrorPeerAccessAlreadyEnabled:
            return "cudaErrorPeerAccessAlreadyEnabled";

          case cudaErrorPeerAccessNotEnabled:
            return "cudaErrorPeerAccessNotEnabled";

          case cudaErrorDeviceAlreadyInUse:
            return "cudaErrorDeviceAlreadyInUse";

          case cudaErrorProfilerDisabled:
            return "cudaErrorProfilerDisabled";

          case cudaErrorProfilerNotInitialized:
            return "cudaErrorProfilerNotInitialized";

          case cudaErrorProfilerAlreadyStarted:
            return "cudaErrorProfilerAlreadyStarted";

          case cudaErrorProfilerAlreadyStopped:
            return "cudaErrorProfilerAlreadyStopped";

#if __CUDA_API_VERSION >= 0x4000

          case cudaErrorAssert:
            return "cudaErrorAssert";

          case cudaErrorTooManyPeers:
            return "cudaErrorTooManyPeers";

          case cudaErrorHostMemoryAlreadyRegistered:
            return "cudaErrorHostMemoryAlreadyRegistered";

          case cudaErrorHostMemoryNotRegistered:
            return "cudaErrorHostMemoryNotRegistered";
#endif

          case cudaErrorStartupFailure:
            return "cudaErrorStartupFailure";

          case cudaErrorApiFailureBase:
            return "cudaErrorApiFailureBase";
          }

        return "<unknown>";
      }
#endif

#ifdef __cuda_cuda_h__
      // CUDA Driver API errors
      static const char *_cudaGetErrorEnum(CUresult error)
      {
        switch (error)
          {
          case CUDA_SUCCESS:
            return "CUDA_SUCCESS";

          case CUDA_ERROR_INVALID_VALUE:
            return "CUDA_ERROR_INVALID_VALUE";

          case CUDA_ERROR_OUT_OF_MEMORY:
            return "CUDA_ERROR_OUT_OF_MEMORY";

          case CUDA_ERROR_NOT_INITIALIZED:
            return "CUDA_ERROR_NOT_INITIALIZED";

          case CUDA_ERROR_DEINITIALIZED:
            return "CUDA_ERROR_DEINITIALIZED";

          case CUDA_ERROR_PROFILER_DISABLED:
            return "CUDA_ERROR_PROFILER_DISABLED";

          case CUDA_ERROR_PROFILER_NOT_INITIALIZED:
            return "CUDA_ERROR_PROFILER_NOT_INITIALIZED";

          case CUDA_ERROR_PROFILER_ALREADY_STARTED:
            return "CUDA_ERROR_PROFILER_ALREADY_STARTED";

          case CUDA_ERROR_PROFILER_ALREADY_STOPPED:
            return "CUDA_ERROR_PROFILER_ALREADY_STOPPED";

          case CUDA_ERROR_NO_DEVICE:
            return "CUDA_ERROR_NO_DEVICE";

          case CUDA_ERROR_INVALID_DEVICE:
            return "CUDA_ERROR_INVALID_DEVICE";

          case CUDA_ERROR_INVALID_IMAGE:
            return "CUDA_ERROR_INVALID_IMAGE";

          case CUDA_ERROR_INVALID_CONTEXT:
            return "CUDA_ERROR_INVALID_CONTEXT";

          case CUDA_ERROR_CONTEXT_ALREADY_CURRENT:
            return "CUDA_ERROR_CONTEXT_ALREADY_CURRENT";

          case CUDA_ERROR_MAP_FAILED:
            return "CUDA_ERROR_MAP_FAILED";

          case CUDA_ERROR_UNMAP_FAILED:
            return "CUDA_ERROR_UNMAP_FAILED";

          case CUDA_ERROR_ARRAY_IS_MAPPED:
            return "CUDA_ERROR_ARRAY_IS_MAPPED";

          case CUDA_ERROR_ALREADY_MAPPED:
            return "CUDA_ERROR_ALREADY_MAPPED";

          case CUDA_ERROR_NO_BINARY_FOR_GPU:
            return "CUDA_ERROR_NO_BINARY_FOR_GPU";

          case CUDA_ERROR_ALREADY_ACQUIRED:
            return "CUDA_ERROR_ALREADY_ACQUIRED";

          case CUDA_ERROR_NOT_MAPPED:
            return "CUDA_ERROR_NOT_MAPPED";

          case CUDA_ERROR_NOT_MAPPED_AS_ARRAY:
            return "CUDA_ERROR_NOT_MAPPED_AS_ARRAY";

          case CUDA_ERROR_NOT_MAPPED_AS_POINTER:
            return "CUDA_ERROR_NOT_MAPPED_AS_POINTER";

          case CUDA_ERROR_ECC_UNCORRECTABLE:
            return "CUDA_ERROR_ECC_UNCORRECTABLE";

          case CUDA_ERROR_UNSUPPORTED_LIMIT:
            return "CUDA_ERROR_UNSUPPORTED_LIMIT";

          case CUDA_ERROR_CONTEXT_ALREADY_IN_USE:
            return "CUDA_ERROR_CONTEXT_ALREADY_IN_USE";

          case CUDA_ERROR_INVALID_SOURCE:
            return "CUDA_ERROR_INVALID_SOURCE";

          case CUDA_ERROR_FILE_NOT_FOUND:
            return "CUDA_ERROR_FILE_NOT_FOUND";

          case CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND:
            return "CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND";

          case CUDA_ERROR_SHARED_OBJECT_INIT_FAILED:
            return "CUDA_ERROR_SHARED_OBJECT_INIT_FAILED";

          case CUDA_ERROR_OPERATING_SYSTEM:
            return "CUDA_ERROR_OPERATING_SYSTEM";

          case CUDA_ERROR_INVALID_HANDLE:
            return "CUDA_ERROR_INVALID_HANDLE";

          case CUDA_ERROR_NOT_FOUND:
            return "CUDA_ERROR_NOT_FOUND";

          case CUDA_ERROR_NOT_READY:
            return "CUDA_ERROR_NOT_READY";

          case CUDA_ERROR_LAUNCH_FAILED:
            return "CUDA_ERROR_LAUNCH_FAILED";

          case CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES:
            return "CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES";

          case CUDA_ERROR_LAUNCH_TIMEOUT:
            return "CUDA_ERROR_LAUNCH_TIMEOUT";

          case CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING:
            return "CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING";

          case CUDA_ERROR_PEER_ACCESS_ALREADY_ENABLED:
            return "CUDA_ERROR_PEER_ACCESS_ALREADY_ENABLED";

          case CUDA_ERROR_PEER_ACCESS_NOT_ENABLED:
            return "CUDA_ERROR_PEER_ACCESS_NOT_ENABLED";

          case CUDA_ERROR_PRIMARY_CONTEXT_ACTIVE:
            return "CUDA_ERROR_PRIMARY_CONTEXT_ACTIVE";

          case CUDA_ERROR_CONTEXT_IS_DESTROYED:
            return "CUDA_ERROR_CONTEXT_IS_DESTROYED";

          case CUDA_ERROR_ASSERT:
            return "CUDA_ERROR_ASSERT";

          case CUDA_ERROR_TOO_MANY_PEERS:
            return "CUDA_ERROR_TOO_MANY_PEERS";

          case CUDA_ERROR_HOST_MEMORY_ALREADY_REGISTERED:
            return "CUDA_ERROR_HOST_MEMORY_ALREADY_REGISTERED";

          case CUDA_ERROR_HOST_MEMORY_NOT_REGISTERED:
            return "CUDA_ERROR_HOST_MEMORY_NOT_REGISTERED";

          case CUDA_ERROR_UNKNOWN:
            return "CUDA_ERROR_UNKNOWN";
          }

        return "<unknown>";
      }
#endif

#ifdef CUBLAS_API_H_
      // cuBLAS API errors
      static const char *_cudaGetErrorEnum(cublasStatus_t error)
      {
        switch (error)
          {
          case CUBLAS_STATUS_SUCCESS:
            return "CUBLAS_STATUS_SUCCESS";

          case CUBLAS_STATUS_NOT_INITIALIZED:
            return "CUBLAS_STATUS_NOT_INITIALIZED";

          case CUBLAS_STATUS_ALLOC_FAILED:
            return "CUBLAS_STATUS_ALLOC_FAILED";

          case CUBLAS_STATUS_INVALID_VALUE:
            return "CUBLAS_STATUS_INVALID_VALUE";

          case CUBLAS_STATUS_ARCH_MISMATCH:
            return "CUBLAS_STATUS_ARCH_MISMATCH";

          case CUBLAS_STATUS_MAPPING_ERROR:
            return "CUBLAS_STATUS_MAPPING_ERROR";

          case CUBLAS_STATUS_EXECUTION_FAILED:
            return "CUBLAS_STATUS_EXECUTION_FAILED";

          case CUBLAS_STATUS_INTERNAL_ERROR:
            return "CUBLAS_STATUS_INTERNAL_ERROR";
          }

        return "<unknown>";
      }
#endif

#ifdef _CUFFT_H_
      // cuFFT API errors
      static const char *_cudaGetErrorEnum(cufftResult error)
      {
        switch (error)
          {
          case CUFFT_SUCCESS:
            return "CUFFT_SUCCESS";

          case CUFFT_INVALID_PLAN:
            return "CUFFT_INVALID_PLAN";

          case CUFFT_ALLOC_FAILED:
            return "CUFFT_ALLOC_FAILED";

          case CUFFT_INVALID_TYPE:
            return "CUFFT_INVALID_TYPE";

          case CUFFT_INVALID_VALUE:
            return "CUFFT_INVALID_VALUE";

          case CUFFT_INTERNAL_ERROR:
            return "CUFFT_INTERNAL_ERROR";

          case CUFFT_EXEC_FAILED:
            return "CUFFT_EXEC_FAILED";

          case CUFFT_SETUP_FAILED:
            return "CUFFT_SETUP_FAILED";

          case CUFFT_INVALID_SIZE:
            return "CUFFT_INVALID_SIZE";

          case CUFFT_UNALIGNED_DATA:
            return "CUFFT_UNALIGNED_DATA";
          }

        return "<unknown>";
      }
#endif


#ifdef CUSPARSEAPI
      // cuSPARSE API errors
      static const char *_cudaGetErrorEnum(cusparseStatus_t error)
      {
        switch (error)
          {
          case CUSPARSE_STATUS_SUCCESS:
            return "CUSPARSE_STATUS_SUCCESS";

          case CUSPARSE_STATUS_NOT_INITIALIZED:
            return "CUSPARSE_STATUS_NOT_INITIALIZED";

          case CUSPARSE_STATUS_ALLOC_FAILED:
            return "CUSPARSE_STATUS_ALLOC_FAILED";

          case CUSPARSE_STATUS_INVALID_VALUE:
            return "CUSPARSE_STATUS_INVALID_VALUE";

          case CUSPARSE_STATUS_ARCH_MISMATCH:
            return "CUSPARSE_STATUS_ARCH_MISMATCH";

          case CUSPARSE_STATUS_MAPPING_ERROR:
            return "CUSPARSE_STATUS_MAPPING_ERROR";

          case CUSPARSE_STATUS_EXECUTION_FAILED:
            return "CUSPARSE_STATUS_EXECUTION_FAILED";

          case CUSPARSE_STATUS_INTERNAL_ERROR:
            return "CUSPARSE_STATUS_INTERNAL_ERROR";

          case CUSPARSE_STATUS_MATRIX_TYPE_NOT_SUPPORTED:
            return "CUSPARSE_STATUS_MATRIX_TYPE_NOT_SUPPORTED";
          }

        return "<unknown>";
      }
#endif

#ifdef CURAND_H_
      // cuRAND API errors
      static const char *_cudaGetErrorEnum(curandStatus_t error)
      {
        switch (error)
          {
          case CURAND_STATUS_SUCCESS:
            return "CURAND_STATUS_SUCCESS";

          case CURAND_STATUS_VERSION_MISMATCH:
            return "CURAND_STATUS_VERSION_MISMATCH";

          case CURAND_STATUS_NOT_INITIALIZED:
            return "CURAND_STATUS_NOT_INITIALIZED";

          case CURAND_STATUS_ALLOCATION_FAILED:
            return "CURAND_STATUS_ALLOCATION_FAILED";

          case CURAND_STATUS_TYPE_ERROR:
            return "CURAND_STATUS_TYPE_ERROR";

          case CURAND_STATUS_OUT_OF_RANGE:
            return "CURAND_STATUS_OUT_OF_RANGE";

          case CURAND_STATUS_LENGTH_NOT_MULTIPLE:
            return "CURAND_STATUS_LENGTH_NOT_MULTIPLE";

          case CURAND_STATUS_DOUBLE_PRECISION_REQUIRED:
            return "CURAND_STATUS_DOUBLE_PRECISION_REQUIRED";

          case CURAND_STATUS_LAUNCH_FAILURE:
            return "CURAND_STATUS_LAUNCH_FAILURE";

          case CURAND_STATUS_PREEXISTING_FAILURE:
            return "CURAND_STATUS_PREEXISTING_FAILURE";

          case CURAND_STATUS_INITIALIZATION_FAILED:
            return "CURAND_STATUS_INITIALIZATION_FAILED";

          case CURAND_STATUS_ARCH_MISMATCH:
            return "CURAND_STATUS_ARCH_MISMATCH";

          case CURAND_STATUS_INTERNAL_ERROR:
            return "CURAND_STATUS_INTERNAL_ERROR";
          }

        return "<unknown>";
      }
#endif

#ifdef NV_NPPIDEFS_H
      // NPP API errors
      static const char *_cudaGetErrorEnum(NppStatus error)
      {
        switch (error)
          {
          case NPP_NOT_SUPPORTED_MODE_ERROR:
            return "NPP_NOT_SUPPORTED_MODE_ERROR";

          case NPP_ROUND_MODE_NOT_SUPPORTED_ERROR:
            return "NPP_ROUND_MODE_NOT_SUPPORTED_ERROR";

          case NPP_RESIZE_NO_OPERATION_ERROR:
            return "NPP_RESIZE_NO_OPERATION_ERROR";

          case NPP_NOT_SUFFICIENT_COMPUTE_CAPABILITY:
            return "NPP_NOT_SUFFICIENT_COMPUTE_CAPABILITY";

#if ((NPP_VERSION_MAJOR << 12) + (NPP_VERSION_MINOR << 4)) <= 0x5000
          case NPP_BAD_ARG_ERROR:
            return "NPP_BAD_ARGUMENT_ERROR";

          case NPP_COEFF_ERROR:
            return "NPP_COEFFICIENT_ERROR";

          case NPP_RECT_ERROR:
            return "NPP_RECTANGLE_ERROR";

          case NPP_QUAD_ERROR:
            return "NPP_QUADRANGLE_ERROR";

          case NPP_MEM_ALLOC_ERR:
            return "NPP_MEMORY_ALLOCATION_ERROR";

          case NPP_HISTO_NUMBER_OF_LEVELS_ERROR:
            return "NPP_HISTOGRAM_NUMBER_OF_LEVELS_ERROR";

          case NPP_INVALID_INPUT:
            return "NPP_INVALID_INPUT";

          case NPP_POINTER_ERROR:
            return "NPP_POINTER_ERROR";

          case NPP_WARNING:
            return "NPP_WARNING";

          case NPP_ODD_ROI_WARNING:
            return "NPP_ODD_ROI_WARNING";
#else

            // These are for CUDA 5.5 or higher
          case NPP_BAD_ARGUMENT_ERROR:
            return "NPP_BAD_ARGUMENT_ERROR";

          case NPP_COEFFICIENT_ERROR:
            return "NPP_COEFFICIENT_ERROR";

          case NPP_RECTANGLE_ERROR:
            return "NPP_RECTANGLE_ERROR";

          case NPP_QUADRANGLE_ERROR:
            return "NPP_QUADRANGLE_ERROR";

          case NPP_MEMORY_ALLOCATION_ERR:
            return "NPP_MEMORY_ALLOCATION_ERROR";

          case NPP_HISTOGRAM_NUMBER_OF_LEVELS_ERROR:
            return "NPP_HISTOGRAM_NUMBER_OF_LEVELS_ERROR";

          case NPP_INVALID_HOST_POINTER_ERROR:
            return "NPP_INVALID_HOST_POINTER_ERROR";

          case NPP_INVALID_DEVICE_POINTER_ERROR:
            return "NPP_INVALID_DEVICE_POINTER_ERROR";
#endif

          case NPP_LUT_NUMBER_OF_LEVELS_ERROR:
            return "NPP_LUT_NUMBER_OF_LEVELS_ERROR";

          case NPP_TEXTURE_BIND_ERROR:
            return "NPP_TEXTURE_BIND_ERROR";

          case NPP_WRONG_INTERSECTION_ROI_ERROR:
            return "NPP_WRONG_INTERSECTION_ROI_ERROR";

          case NPP_NOT_EVEN_STEP_ERROR:
            return "NPP_NOT_EVEN_STEP_ERROR";

          case NPP_INTERPOLATION_ERROR:
            return "NPP_INTERPOLATION_ERROR";

          case NPP_RESIZE_FACTOR_ERROR:
            return "NPP_RESIZE_FACTOR_ERROR";

          case NPP_HAAR_CLASSIFIER_PIXEL_MATCH_ERROR:
            return "NPP_HAAR_CLASSIFIER_PIXEL_MATCH_ERROR";


#if ((NPP_VERSION_MAJOR << 12) + (NPP_VERSION_MINOR << 4)) <= 0x5000
          case NPP_MEMFREE_ERR:
            return "NPP_MEMFREE_ERR";

          case NPP_MEMSET_ERR:
            return "NPP_MEMSET_ERR";

          case NPP_MEMCPY_ERR:
            return "NPP_MEMCPY_ERROR";

          case NPP_MIRROR_FLIP_ERR:
            return "NPP_MIRROR_FLIP_ERR";
#else
          case NPP_MEMFREE_ERROR:
            return "NPP_MEMFREE_ERROR";

          case NPP_MEMSET_ERROR:
            return "NPP_MEMSET_ERROR";

          case NPP_MEMCPY_ERROR:
            return "NPP_MEMCPY_ERROR";

          case NPP_MIRROR_FLIP_ERROR:
            return "NPP_MIRROR_FLIP_ERROR";
#endif

          case NPP_ALIGNMENT_ERROR:
            return "NPP_ALIGNMENT_ERROR";

          case NPP_STEP_ERROR:
            return "NPP_STEP_ERROR";

          case NPP_SIZE_ERROR:
            return "NPP_SIZE_ERROR";

          case NPP_NULL_POINTER_ERROR:
            return "NPP_NULL_POINTER_ERROR";

          case NPP_CUDA_KERNEL_EXECUTION_ERROR:
            return "NPP_CUDA_KERNEL_EXECUTION_ERROR";

          case NPP_NOT_IMPLEMENTED_ERROR:
            return "NPP_NOT_IMPLEMENTED_ERROR";

          case NPP_ERROR:
            return "NPP_ERROR";

          case NPP_SUCCESS:
            return "NPP_SUCCESS";

          case NPP_WRONG_INTERSECTION_QUAD_WARNING:
            return "NPP_WRONG_INTERSECTION_QUAD_WARNING";

          case NPP_MISALIGNED_DST_ROI_WARNING:
            return "NPP_MISALIGNED_DST_ROI_WARNING";

          case NPP_AFFINE_QUAD_INCORRECT_WARNING:
            return "NPP_AFFINE_QUAD_INCORRECT_WARNING";

          case NPP_DOUBLE_SIZE_WARNING:
            return "NPP_DOUBLE_SIZE_WARNING";

          case NPP_WRONG_INTERSECTION_ROI_WARNING:
            return "NPP_WRONG_INTERSECTION_ROI_WARNING";
          }

        return "<unknown>";
      }
#endif



#ifdef __DRIVER_TYPES_H__
#ifndef IMP_DEVICE_RESET
#define IMP_DEVICE_RESET cudaDeviceReset();
#endif
#else
#ifndef IMP_DEVICE_RESET
#define IMP_DEVICE_RESET
#endif
#endif

      template< typename T >
        void check(T result, char const *const func, const char *const file, int const line)
        {
          if (result)
            {
              fprintf(stderr, "CUDA error at %s:%d code=%d(%s) \"%s\" \n",
                      file, line, static_cast<unsigned int>(result), _cudaGetErrorEnum(result), func);
              IMP_DEVICE_RESET
                // Make sure we call CUDA Device Reset before exiting
                exit(-1);
            }
        }

#ifdef __DRIVER_TYPES_H__
      // This will output the proper CUDA error strings in the event that a CUDA host call returns an error
#define IMP_checkCudaErrors(val)           check ( (val), #val, __FILE__, __LINE__ )

      // This will output the proper error string when calling cudaGetLastError
#define IMP_getLastCudaError(msg)      __getLastCudaError (msg, __FILE__, __LINE__)

      inline void __getLastCudaError(const char *errorMessage, const char *file, const int line)
      {
        cudaError_t err = cudaGetLastError();

        if (cudaSuccess != err)
          {
            fprintf(stderr, "%s(%i) : getLastCudaError() CUDA error : %s : (%d) %s.\n",
                    file, line, errorMessage, (int)err, cudaGetErrorString(err));
            IMP_DEVICE_RESET
              exit(EXIT_FAILURE);
          }
      }
#endif

      // // utility method for checking CUDA rt-errors
      // #define checkCudaErrors(msg) \
      //     do { \
      //         cudaError_t __err = cudaGetLastError(); \
      //         if (__err != cudaSuccess) { \
      //             fprintf(stderr, "Fatal error: %s (%s at %s:%d)\n", \
      //                 msg, cudaGetErrorString(__err), \
      //                 __FILE__, __LINE__); \
      //             fprintf(stderr, "*** FAILED - ABORTING\n"); \
      //             exit(1); \
      //         } \
      //     } while (0)


    };
  };
};
#endif /* IMPKERNEL_INTERNAL_CUDA_HELPER_FUNCTIONS_H */
