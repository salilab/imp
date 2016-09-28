/**
 *  \file mpi_helpers.h
 *  \brief Helper functions to set up MPI.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 */

#ifndef IMPMPI_MPI_HELPERS_H
#define IMPMPI_MPI_HELPERS_H

#include <IMP/mpi/mpi_config.h>

IMPMPI_BEGIN_INTERNAL_NAMESPACE

//! Set up the MPI communicator.
/** This takes care of calling MPI_Init now, and MPI_Finalize at the end of
    the process. */
IMPMPIEXPORT void setup_mpi_communicator();

IMPMPI_END_INTERNAL_NAMESPACE

#endif /* IMPMPI_MPI_HELPERS_H */
