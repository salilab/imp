/**
 *  \file ReplicaExchange.h
 *  \brief A class to do replica exchange in a generic hamiltonian parameter
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPSPB_DEPRECATED_REPLICA_EXCHANGE_H
#define IMPSPB_DEPRECATED_REPLICA_EXCHANGE_H

#include "spb_config.h"
#include <IMP/base/Object.h>
#include "mpi.h"
#include <string>

IMPSPB_BEGIN_NAMESPACE

// Replica Exchange
class IMPSPBEXPORT ReplicaExchange: public base::Object
{
  int myrank_,nproc_;
  Ints index_;
  Ints exarray_;
  MPI_Status status_;
  std::map<std::string,Floats> parameters_;

private:
  Ints   create_indexes();
  Ints   create_exarray();
  bool   get_acceptance(double myscore,double fscore);
  int    get_rank(int index);

public:
  ReplicaExchange();
  void   set_my_parameter(std::string key,Floats values);
  Floats get_my_parameter(std::string key);
  int    get_friend_index(int istep);
  Ints   get_exchange_array();
  Floats get_friend_parameter(std::string key,int findex);
  bool   do_exchange(double myscore0,double myscore1,int findex);
  Floats create_temperatures(double tmin,double tmax,int nrep);

  int get_my_index() const {
   return index_[myrank_];
  }

  int get_number_of_replicas() const {
   return nproc_;
  }

  IMP_OBJECT_INLINE(ReplicaExchange,,MPI_Finalize());

};

IMPSPB_END_NAMESPACE

#endif  /* IMPSPB_DEPRECATED_REPLICA_EXCHANGE_H */
