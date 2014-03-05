/**
 *  \file ReplicaExchange.cpp
 *  \brief Replica Exchange
 *
 *  Copyright 2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/mpi/ReplicaExchange.h>
#include "mpi.h"
#include <boost/scoped_array.hpp>
#include <time.h>

IMPMPI_BEGIN_NAMESPACE

ReplicaExchange::ReplicaExchange() : Object("Replica Exchange") {
  // These are needed to initialize the MPI communicator
  MPI::Init();
  MPI_Comm_size(MPI_COMM_WORLD, &nproc_);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank_);
  // Create the list of replica indexes
  index_ = create_indexes();
  // and list to keep track of the exchange acceptance
  exarray_ = create_exarray();
  // initialize seed
  unsigned int iseed = time(NULL)+myrank_;
  srand(iseed);
}

Ints ReplicaExchange::create_indexes() {
  Ints index;
  for (int i = 0; i < nproc_; ++i) {
    index.push_back(i);
  }
  return index;
}

Ints ReplicaExchange::create_exarray() {
  Ints exarray;
  for (int i = 0; i < nproc_ - 1; ++i) {
    exarray.push_back(0);
  }
  return exarray;
}

void ReplicaExchange::set_my_parameter(std::string key, Floats values) {
  parameters_[key] = values;
}

Floats ReplicaExchange::get_my_parameter(std::string key) {
  return parameters_[key];
}

int ReplicaExchange::get_friend_index(int step) {
  int myindex = index_[myrank_];
  int findex = -1;
  if (myindex % 2 == 0 && step % 2 == 0) {
    findex = myindex + 1;
  }
  if (myindex % 2 == 0 && step % 2 == 1) {
    findex = myindex - 1;
  }
  if (myindex % 2 == 1 && step % 2 == 0) {
    findex = myindex - 1;
  }
  if (myindex % 2 == 1 && step % 2 == 1) {
    findex = myindex + 1;
  }
  if (findex == -1) {
    findex = nproc_ - 1;
  }
  if (findex == nproc_) {
    findex = 0;
  }
  return findex;
}

int ReplicaExchange::get_rank(int index) {
  int rank = 0;
  for (int i = 0; i < nproc_; ++i) {
    if (index_[i] == index) {
      rank = i;
    }
  }
  return rank;
}

Floats ReplicaExchange::get_friend_parameter(std::string key, int findex) {
  int frank = get_rank(findex);
  int nparam = parameters_[key].size();
  double* myparameters = new double[nparam];
  std::copy(parameters_[key].begin(), parameters_[key].end(), myparameters);

  double* fparameters = new double[nparam];

  MPI_Sendrecv(myparameters, nparam, MPI_DOUBLE, frank, myrank_, fparameters,
               nparam, MPI_DOUBLE, frank, frank, MPI_COMM_WORLD, &status_);

  Floats fpar(fparameters, fparameters + nparam);
  delete[] myparameters;
  delete[] fparameters;

  return fpar;
}

Ints ReplicaExchange::get_exchange_array() { return exarray_; }

bool ReplicaExchange::do_exchange(double myscore0, double myscore1,
                                  int findex) {
  double myscore = myscore0 - myscore1;
  int frank = get_rank(findex);
  bool do_accept;
  //lowest index does the exchange,
  //the other one sends his parameters and receives the result
  if (myrank_ < frank) {
      //we do the exchange
      double fscore;

      MPI_Recv(&fscore, 1, MPI_DOUBLE, frank, 0, MPI_COMM_WORLD, &status_);
      do_accept = get_acceptance(myscore, fscore);
      int atmp = (int)do_accept;
      MPI_Send(&atmp, 1, MPI_INT, frank, 0, MPI_COMM_WORLD);
  } else {
      MPI_Send(&myscore, 1, MPI_DOUBLE, frank, 0, MPI_COMM_WORLD);
      int atmp;
      MPI_Recv(&atmp, 1, MPI_INT, frank, 0, MPI_COMM_WORLD, &status_);
      do_accept = (bool)atmp;
  }

  boost::scoped_array<int> sdel(new int[nproc_ - 1]);
  boost::scoped_array<int> rdel(new int[nproc_ - 1]);

  for (int i = 0; i < nproc_ - 1; ++i) {
    sdel[i] = 0;
  }

  int myindex = index_[myrank_];
  if (do_accept) {
    std::map<std::string, Floats>::iterator it;
    for (it = parameters_.begin(); it != parameters_.end(); it++) {
      Floats param = get_friend_parameter((*it).first, findex);
      set_my_parameter((*it).first, param);
    }
    // update the increment vector only to those replicas that upgraded to
    // a higher temperature to avoid double
    // calculations (excluding the transition 0 -> nrep-1)
    int delindex = findex - myindex;
    if (delindex == 1) {
      // std::cout << myindex << " " << findex << " " << std::endl;
      sdel[myindex] = 1;
    }
    // update the indexes
    myindex = findex;
  }

  MPI_Barrier(MPI_COMM_WORLD);
  // get the increment vector from all replicas and copy it to the
  // exchange array
  MPI_Allreduce(sdel.get(), rdel.get(), nproc_ - 1, MPI_INT, MPI_SUM,
                MPI_COMM_WORLD);
  for (int i = 0; i < nproc_ - 1; ++i) {
    exarray_[i] = rdel[i];
  }
  // in any case, update index vector
  boost::scoped_array<int> sbuf(new int[nproc_]);
  boost::scoped_array<int> rbuf(new int[nproc_]);
  for (int i = 0; i < nproc_; ++i) {
    sbuf[i] = 0;
  }
  sbuf[myrank_] = myindex;
  MPI_Allreduce(sbuf.get(), rbuf.get(), nproc_, MPI_INT, MPI_SUM,
                MPI_COMM_WORLD);
  for (int i = 0; i < nproc_; ++i) {
    index_[i] = rbuf[i];
  }
  return do_accept;
}

bool ReplicaExchange::get_acceptance(double myscore, double fscore) {
  // log file
  double accept, delta;
  delta = myscore + fscore;

  if (delta >= 0.0) {
    accept = 1.0;
  } else {
    accept = exp(delta);
  }
  double random = (double)rand() / (double)RAND_MAX;

  if (random <= accept) {
    return true;
  } else {
    return false;
  }
}

Floats ReplicaExchange::create_temperatures(double tmin, double tmax,
                                            int nrep) {
  Floats temp;
  double tfact = exp(log(tmax / tmin) / double(nrep - 1));
  for (int i = 0; i < nrep; ++i) {
    temp.push_back(tmin * pow(tfact, i));
  }
  return temp;
}

IMPMPI_END_NAMESPACE
