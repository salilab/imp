The MPI module is a place for code that requires MPI.

Note that the default build of OpenMPI on many Linux systems doesn't play nicely
with Python (you may see errors like `mca: base: component_find: unable to
open /usr/lib64/openmpi/lib/openmpi/mca_shmem_sysv: perhaps a missing symbol,
or compiled for a different version of Open MPI?`. This can be fixed either
by recompiling OpenMPI with the `--disable-dlopen` configure flag, or by
running `import DLFCN as dl; sys.setdlopenflags(dl.RTLD_NOW|dl.RTLD_GLOBAL)`
in your Python script before importing IMP.mpi.

# Info

_Author(s)_:

_Maintainer_: `benmwebb`

_License_: LGPL

_Publications_:
- None
