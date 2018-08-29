\brief Code that uses the MPI parallel library.

To enable this module, you must first install an MPI library and then make sure
that the `mpic++` binary is in the PATH (e.g. on a RedHat/Fedora system, you
may need to run `module load mpich` or similar) before you
run [cmake](@ref cmake_config). (There is no need to build
all of IMP with `mpic++` - that would force all of IMP to need the MPI library,
not just the IMP.mpi module.)

If you want to use [OpenMPI](http://www.open-mpi.org/), note that the default
build on many Linux systems doesn't play nicely
with Python (you may see errors like `mca: base: component_find: unable to open /usr/lib64/openmpi/lib/openmpi/mca_shmem_sysv: perhaps a missing symbol, or compiled for a different version of Open MPI?`). This can be fixed either
by recompiling OpenMPI with the `--disable-dlopen` configure flag, or by
running `import DLFCN as dl; sys.setdlopenflags(dl.RTLD_NOW|dl.RTLD_GLOBAL)`
in your Python script before importing IMP.mpi. We also recommend that you
use version 1.7 of OpenMPI or older, as the 1.8 release does not appear to
work with the [mpi4py](http://mpi4py.scipy.org/docs/) package that some parts
of IMP::pmi use.

# Info

_Author(s)_: Max Bonomi

_Maintainer_: `benmwebb`

_License_: [LGPL](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See [main IMP papers list](@ref publications).
