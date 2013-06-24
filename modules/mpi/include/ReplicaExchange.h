/**
 *  \file ReplicaExchange.h
 *  \brief A class to do replica exchange in a generic hamiltonian parameter
 *
 *  Copyright 2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPMPI_REPLICA_EXCHANGE_H
#define IMPMPI_REPLICA_EXCHANGE_H

#include "mpi_config.h"
#include <IMP/Object.h>
#include "mpi.h"
#include <string>

IMPMPI_BEGIN_NAMESPACE

//! A class to implement Hamiltonian Replica Exchange
/** Any model parameter can be used as exchange parameter:
    temperature, restraint intensity, ...
*/
class IMPMPIEXPORT ReplicaExchange: public base::Object
{
  //! MPI rank
  int myrank_;
  //! Number of replicas
  int nproc_;
  //! List of replica indexes
  Ints index_;
  //! List of number of exchanges accepted
  Ints exarray_;
  //! MPI status
  MPI_Status status_;
  //! Exchange parameters
  std::map<std::string,Floats> parameters_;

private:
  //! Create list of replica indices
  Ints   create_indexes();
  //! Create list of number of exchange accepted
  /** This is useful to track exchange acceptance across replicas */
  Ints   create_exarray();
  //! Calculate acceptance probability
  bool   get_acceptance(double myscore,double fscore);
  //! Get the rank for a given replica index
  int    get_rank(int index);

public:
  ReplicaExchange();
  //! Set exchange parameter
  void   set_my_parameter(std::string key,Floats values);
  //! Get the value of my exchange parameter
  Floats get_my_parameter(std::string key);
  //! Get the index of friend with which trying an exchange
  int    get_friend_index(int istep);
  //! Get the array with exchange acceptance
  Ints   get_exchange_array();
  //! Get the value of my friend's parameter
  Floats get_friend_parameter(std::string key,int findex);
  //! Try an exchange
  bool   do_exchange(double myscore0,double myscore1,int findex);
  //! Initialize array of temperatures
  /** Temperatures are distributed according to a geometric progression */
  Floats create_temperatures(double tmin,double tmax,int nrep);
  //! Get replica index
  int get_my_index() const {return index_[myrank_];}
  //! Get number of replicas
  int get_number_of_replicas() const {return nproc_;}

  IMP_OBJECT_INLINE(ReplicaExchange,,MPI_Finalize());

};

IMPMPI_END_NAMESPACE

#endif  /* IMPMPI_REPLICA_EXCHANGE_H */
