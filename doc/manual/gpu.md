GPU support {#gpu}
===========

%IMP currently has rudimentary support for running on a graphics
processing unit (GPU) or similar systems such as Tensor Processing Units (TPUs).
This support uses the [JAX](https://docs.jax.dev/) Python library.

To use the JAX support in optimization, first install the JAX library,
for example with `pip install jax`. Then set up the system as per usual and
replace any calls to IMP::core::MonteCarlo::optimize or
IMP::atom::MolecularDynamics::optimize with `_optimize_jax()`.

The JAX code is still in active development and many caveats apply:

 - Only a small number of scoring function terms and optimizers currently
   have JAX implementations. Trying to use others will result in a
   Python NotImplementedError exception.
 - Some IMP::ScoreState (aka constraint) classes do not yet work - this
   includes common applications such as rigid bodies and close pair containers.
 - There is currently no PMI support for JAX.

To add JAX support for a particular IMP::Restraint, IMP::PairScore,
IMP::core::MonteCarloMover, or IMP::OptimizerState,
implement the `_get_jax()` method. See the
[IMP.example module](https://github.com/salilab/imp/blob/develop/modules/example/pyext/IMP_example.jax.i)
for some examples

Note that the JAX code will also run on a CPU. In some circumstances the
JAX code will run faster than the native %IMP C++ code on a CPU, so it may
be worth benchmarking both approaches.

Note that %IMP also has some very basic C++ support for NVIDIA GPUs using the
CUDA toolkit. This is unlikely to be further developed, however.
To build %IMP from source code with CUDA support (there are currently no
prebuilt %IMP binaries that use CUDA), ensure that the `nvcc` compiler
from NVIDIA's [CUDA toolkit](https://developer.nvidia.com/cuda-downloads)
is available, and add `-DIMP_CUDA` to your
[CMake invocation](@ref cmake_config).
