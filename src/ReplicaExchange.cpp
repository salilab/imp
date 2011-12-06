/**
 *  \file ReplicaExchange.cpp
 *  \brief Replica Exchange
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane/ReplicaExchange.h>
#include "mpi.h"
#include <iostream>
#include <string>
#include <time.h>

IMPMEMBRANE_BEGIN_NAMESPACE

ReplicaExchange::ReplicaExchange(): Object("Replica Exchange")
{
 MPI::Init();
 MPI_Comm_size(MPI_COMM_WORLD, &nproc_);
 MPI_Comm_rank(MPI_COMM_WORLD, &myrank_);
 index_=create_indexes();
// initialize seed
 unsigned int iseed = time(NULL);
 // broadcast seed
 MPI_Bcast(&iseed,1,MPI_UNSIGNED,0,MPI_COMM_WORLD);
// initialize random generator
 srand (iseed);
}

Ints ReplicaExchange::create_indexes()
{
 Ints index;
 for(int i=0;i<nproc_;++i){index.push_back(i);}
 return index;
}

void ReplicaExchange::set_my_parameter(std::string key, Floats values)
{
 parameters_[key]=values;
}

Floats ReplicaExchange::get_my_parameter(std::string key)
{
 return parameters_[key];
}

int ReplicaExchange::get_friend_index(int step)
{
 int myindex=index_[myrank_];
 int findex;
 if(myindex%2==0 && step%2==0) {findex=myindex+1;}
 if(myindex%2==0 && step%2==1) {findex=myindex-1;}
 if(myindex%2==1 && step%2==0) {findex=myindex-1;}
 if(myindex%2==1 && step%2==1) {findex=myindex+1;}
 if(findex==-1)     {findex=nproc_-1;}
 if(findex==nproc_) {findex=0;}
 return findex;
}

int ReplicaExchange::get_rank(int index)
{
 int rank;
 for(int i=0;i<nproc_;++i) {if(index_[i]==index) {rank=i;}}
 return rank;
}

Floats ReplicaExchange::get_friend_parameter(std::string key, int findex)
{
 int frank=get_rank(findex);
 int nparam=parameters_[key].size();
 double* myparameters=&(parameters_[key])[0];
 double* fparameters=new double[nparam];

 MPI_Sendrecv(myparameters,nparam,MPI_DOUBLE,frank,myrank_,
               fparameters,nparam,MPI_DOUBLE,frank,frank,
               MPI_COMM_WORLD,&status_);

 Floats fpar(fparameters,fparameters+nparam);
 delete(fparameters);
 return fpar;
}

bool ReplicaExchange::do_exchange(double myscore0, double myscore1, int findex)
{
 double myscore=myscore0-myscore1;
 double fscore;
 int myindex=index_[myrank_];
 int frank=get_rank(findex);

 MPI_Sendrecv(&myscore,1,MPI_DOUBLE,frank,myrank_,
               &fscore,1,MPI_DOUBLE,frank,frank,
                MPI_COMM_WORLD, &status_);

 bool do_accept=get_acceptance(myscore,fscore);

 if(do_accept){
  std::map<std::string,Floats>::iterator it;
  for (it = parameters_.begin(); it != parameters_.end(); it++){
   Floats par=get_friend_parameter((*it).first,findex);
   set_my_parameter((*it).first,par);
  }
  myindex=findex;
 }

 // in any case, update index vector
 MPI_Barrier(MPI_COMM_WORLD);
 int sbuf[nproc_],rbuf[nproc_];
 for(int i=0;i<nproc_;++i) {sbuf[i]=0;}
 sbuf[myrank_]=myindex;
 MPI_Allreduce(sbuf,rbuf,nproc_,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
 for(int i=0;i<nproc_;++i){index_[i]=rbuf[i];}

 return do_accept;
}

bool ReplicaExchange::get_acceptance(double myscore,double fscore)
{
 double accept,delta;
 delta=myscore+fscore;
 if(delta>=0.0){
  accept=1.0;
 }else{
  accept=exp(delta);
 }
 double random= (double) rand()/RAND_MAX;
 if(random<=accept){
  return true;
 }else{
  return false;
 }
}

Floats ReplicaExchange::create_temperatures(double tmin,double tmax,int nrep)
{
 Floats temp;
 double tfact=exp(log(tmax/tmin)/double(nrep-1));
 for(int i=0;i<nrep;++i) {
  temp.push_back(tmin*pow(tfact,i));
 }
 return temp;
}

IMPMEMBRANE_END_NAMESPACE
