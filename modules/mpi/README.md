\brief Code that uses the MPI parallel library.

To enable this module, you must first install an MPI library and then make sure
that the `mpic++` binary is in the PATH (e.g. on a RedHat/Fedora system, you
may need to run `module load mpich` or similar) before you
run [cmake](@ref cmake_config). (There is no need to build
all of IMP with `mpic++` - that would force all of IMP to need the MPI library,
not just the IMP.mpi module.)

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
