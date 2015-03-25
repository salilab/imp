Parallelization {#parallel}
===============

There are several ways to run calculations in parallel in %IMP:

 - IMP.mpi module. This is used to run tightly-coupled calculations in parallel
   using MPI, over processors that are not necessarily on the same machine.
   In particular, sampling protocols in the IMP.pmi module use this
   where available. %IMP must be compiled with your MPI compiler to use this
   module. See IMP.mpi for more details.

 - IMP.parallel module. This is designed to manage multiple independent tasks
   spread over a pool of available workers. The tasks cannot communicate with 
   each other, but the system is fault tolerant and will restart any failed
   tasks. This module is available in all %IMP builds.
   See IMP.parallel for details.

 - OpenMP support. This is used to spread out %IMP's computation between
   multiple threads on a single multi-core machine. %IMP must be compiled
   with OpenMP support to use this, but when enabled it is used automatically
   by all parts of %IMP that have been parallelized. See the
   [OpenMP](@ref openmp) page for more details.
