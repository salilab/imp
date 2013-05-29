/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <iostream>
#include <mpi.h>

static int numprocs;

int main(int argc, char **argv) {
  int my_rank;
  // MPI initializations
  MPI_Status status;
  MPI_Init (&argc, &argv);
  MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank (MPI_COMM_WORLD, &my_rank);
  double time_start = MPI_Wtime();
  std::cout << "Hello World, my rank is " << my_rank
            << " " << MPI_Wtime() - time_start << std::endl;
  // End MPI
  MPI_Finalize ();
  return 0;
}
