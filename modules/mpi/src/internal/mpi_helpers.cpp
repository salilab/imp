/**
 *  \file mpi_helpers.cpp
 *  \brief Helper functions to set up MPI.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#include <IMP/mpi/internal/mpi_helpers.h>
#include "mpi.h"

IMPMPI_BEGIN_INTERNAL_NAMESPACE

namespace {
  struct MPICommunicator {
    MPICommunicator() {
      int ret;
      MPI_Initialized(&ret);
      if (!ret) {
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
