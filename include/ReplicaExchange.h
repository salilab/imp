/**
 *  \file ReplicaExchange.h
 *  \brief Replica Exchange
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_REPLICA_EXCHANGE_H
#define IMPMEMBRANE_REPLICA_EXCHANGE_H

#include "membrane_config.h"
#include "mpi.h"

IMPMEMBRANE_BEGIN_NAMESPACE

// rem?
class IMPMEMBRANEEXPORT ReplicaExchange
{
  double tmin_,tmax_;
  int myrank_,frank_,nproc_;
  Floats temp_;
  Ints index_;
  MPI_Status status_;

private:
  Floats  create_temperatures(double tmin,double tmax,int nrep);
  Ints    create_indexes(int nrep);
  int     get_friend_rank(Ints index,int myrank,int step,int nrep);
  bool    get_acceptance(double scores[2],double fscores[2],
                         double mytemp, double ftemp);

public:
  ReplicaExchange(double tmin, double tmax);
  ReplicaExchange(Floats temperatures);
  int get_friend_index(int istep);
  int do_exchange(double myscore, double fscore);
  ~ReplicaExchange(){MPI_Finalize();};

  int get_whoiam() const {
   return index_[myrank_];
  }

};

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_REPLICA_EXCHANGE_H */
