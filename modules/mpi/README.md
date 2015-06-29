\brief Code that uses the MPI parallel library.

To enable this module, you must build IMP with your MPI library's MPI compiler
rather than the default C++ compiler (e.g. `mpic++` rather than `g++`). To do
this, first set up your MPI library so that `mpic++` is in your PATH (on some
systems by using something like `module load mpi/openmpi-x86_64`) and then
run [cmake](@ref cmake_config) with the
`-DCMAKE_CXX_COMPILER=mpic++` option.

If you want to use [OpenMPI](http://www.open-mpi.org/), note that the default
build on many Linux systems doesn't play nicely
with Python (you may see errors like `mca: base: component_find: unable to open /usr/lib64/openmpi/lib/openmpi/mca_shmem_sysv: perhaps a missing symbol, or compiled for a different version of Open MPI?`). This can be fixed either
by recompiling OpenMPI with the `--disable-dlopen` configure flag, or by
running `import DLFCN as dl; sys.setdlopenflags(dl.RTLD_NOW|dl.RTLD_GLOBAL)`
in your Python script before importing IMP.mpi. We also recommend that you
use version 1.7 of OpenMPI or older, as the 1.8 release does not appear to
work with the [mpi4py](http://mpi4py.scipy.org/) package that some parts
of IMP::pmi use.

# Info

_Author(s)_: Max Bonomi

_Maintainer_: `benmwebb`

_License_: LGPL

_Publications_:
 - See [main IMP papers list](@ref publications).
