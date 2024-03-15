GPU support {#gpu}
===========

%IMP currently has only very rudimentary support for running on a graphics
processing unit (GPU), although this is currently in development.

%IMP currently only supports NVIDIA GPUs using the CUDA toolkit (although this
is likely to switch to use [SYCL](https://sycl.tech/) as the ecosystem matures).
To build %IMP from source code with CUDA support (there are currently no
prebuilt %IMP binaries that use CUDA), ensure that the `nvcc` compiler
from NVIDIA's [CUDA toolkit](https://developer.nvidia.com/cuda-downloads)
is available, and add `-DIMP_CUDA` to your
[CMake invocation](@ref cmake_config).
