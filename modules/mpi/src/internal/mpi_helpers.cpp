/**
 *  \file mpi_helpers.cpp
 *  \brief Helper functions to set up MPI.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 */

#include <IMP/mpi/internal/mpi_helpers.h>
#include "mpi.h"

#if defined(OMPI_MAJOR_VERSION) && !defined(_MSC_VER)
# include <dlfcn.h>
#endif

IMPMPI_BEGIN_INTERNAL_NAMESPACE

namespace {

/* The code below is a hack to work around an OpenMPI issue. When using OpenMPI
   in combination with dynamic library loading (e.g. Python) it will often
   fail with errors such as

   mca: base: component_find: unable to open
   /usr/lib64/openmpi/lib/openmpi/mca_shmem_sysv: perhaps a missing symbol,
   or compiled for a different version of Open MPI?

   To resolve this we preload the MPI dynamic library with suitable flags to
   ensure library symbols are globally available. This is similar to the code
   from mpi4py at
   https://github.com/mpi4py/mpi4py/blob/master/src/lib-mpi/compat/openmpi.h

   This code will likely need to be updated for newer versions of OpenMPI.
 */
#if defined(OMPI_MAJOR_VERSION) && !defined(_MSC_VER)
  void ompi_dlopen(const std::string &sover) {
    int mode = RTLD_NOW | RTLD_GLOBAL;
#ifdef RTLD_NOLOAD
    mode |= RTLD_NOLOAD;
#endif
#if defined(__APPLE__)
    std::string dlname = "libmpi." + sover + ".dylib";
#else
    std::string dlname = "libmpi.so." + sover;
#endif
    dlopen(dlname.c_str(), mode);
  }
#endif

  void dlopen_libmpi() {
#if defined(OMPI_MAJOR_VERSION) && !defined(_MSC_VER)
#if OMPI_MAJOR_VERSION == 3
    ompi_dlopen("40");
#elif OMPI_MAJOR_VERSION == 2
    ompi_dlopen("20");
#elif OMPI_MAJOR_VERSION == 1 && OMPI_MINOR_VERSION >= 10
    ompi_dlopen("12");
#elif OMPI_MAJOR_VERSION == 1 && OMPI_MINOR_VERSION >= 6
    ompi_dlopen("1");
#elif OMPI_MAJOR_VERSION == 1
    ompi_dlopen("0");
#endif
#endif
  }

  struct MPICommunicator {
    MPICommunicator() {
      int ret;
      MPI_Initialized(&ret);
      if (!ret) {
        dlopen_libmpi();
        MPI_Init(0, 0);
      }
    }

    ~MPICommunicator() {
      int ret;
      MPI_Finalized(&ret);
      if (!ret) {
        MPI_Finalize();
      }
    }
  };
}

void setup_mpi_communicator() {
  // m will be constructed on the first call, and then not destroyed until
  // the process ends
  static MPICommunicator m;
};

IMPMPI_END_INTERNAL_NAMESPACE
