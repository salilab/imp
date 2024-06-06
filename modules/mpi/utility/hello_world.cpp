/**
 * Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#include <iostream>

// We only want the C API, so try to suppress the C++ API
#ifndef MPICH_SKIP_MPICXX
#define MPICH_SKIP_MPICXX
#endif
#ifndef OMPI_SKIP_MPICXX
#define OMPI_SKIP_MPICXX
#endif
#ifndef _MPICC_H
#define _MPICC_H
#endif
#include <mpi.h>

static int numprocs;

int main(int argc, char **argv) {
  int my_rank;
  // MPI initializations
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  double time_start = MPI_Wtime();
  std::cout << "Hello World, my rank is " << my_rank << " "
            << MPI_Wtime() - time_start << std::endl;
  // End MPI
  MPI_Finalize();
  return 0;
}
