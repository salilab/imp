/**
 *  \file ReplicaExchange.cpp
 *  \brief Replica Exchange
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane/ReplicaExchange.h>
#include <IMP/core.h>
#include "mpi.h"
#include <iostream>

IMPMEMBRANE_BEGIN_NAMESPACE

ReplicaExchange::ReplicaExchange(double tmin, double tmax){
 tmin_=tmin;
 tmax_=tmax;
 MPI::Init();
 MPI_Comm_size(MPI_COMM_WORLD, &nproc_);
 MPI_Comm_rank(MPI_COMM_WORLD, &myrank_);
 temp_=create_temperatures(tmin_,tmax_,nproc_);
 index_=create_indexes(nproc_);
}

ReplicaExchange::ReplicaExchange(Floats temperatures){
 temp_=temperatures;
 MPI::Init();
 MPI_Comm_size(MPI_COMM_WORLD, &nproc_);
 MPI_Comm_rank(MPI_COMM_WORLD, &myrank_);
 index_=create_indexes(nproc_);
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

Ints ReplicaExchange::create_indexes(int nrep)
{
 Ints index;
 for(int i=0;i<nrep;++i) {
  index.push_back(i);
 }
 return index;
}

int ReplicaExchange::get_friend_index(int istep)
{
 frank_=get_friend_rank(index_,myrank_,istep,nproc_);
 return index_[frank_];
}

int ReplicaExchange::do_exchange(double myscore, double fscore)
{
 double scores[2]={myscore,fscore};
 double fscores[2];
 int myindex=index_[myrank_];
 int findex=index_[frank_];

 MPI_Sendrecv(scores,2,MPI_DOUBLE,frank_,myrank_,
              fscores,2,MPI_DOUBLE,frank_,frank_,
                MPI_COMM_WORLD, &status_);

 // calculate acceptance
 bool do_accept=get_acceptance(scores,fscores,temp_[myindex],temp_[findex]);

 if(do_accept){myindex=findex;}

 // in any case, update index vector
 MPI_Barrier(MPI_COMM_WORLD);
 int sbuf[nproc_],rbuf[nproc_];
 for(int i=0;i<nproc_;++i) {sbuf[i]=0;}
 sbuf[myrank_]=myindex;
 MPI_Allreduce(sbuf,rbuf,nproc_,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
 for(int i=0;i<nproc_;++i){index_[i]=rbuf[i];}

 return myindex;
}

int ReplicaExchange::get_friend_rank(Ints index,int myrank,int step,int nrep)
{
 int myindex=index[myrank];
 int findex,frank;

 if(myindex%2==0 && step%2==0) {findex=myindex+1;}
 if(myindex%2==0 && step%2==1) {findex=myindex-1;}
 if(myindex%2==1 && step%2==0) {findex=myindex-1;}
 if(myindex%2==1 && step%2==1) {findex=myindex+1;}
 if(findex==-1)   {findex=nrep-1;}
 if(findex==nrep) {findex=0;}

 for(int i=0; i<nrep; ++i) {if(index[i]==findex) {frank=i;}}
 return frank;
}


bool ReplicaExchange::get_acceptance(double scores[2],double fscores[2],
                         double mytemp, double ftemp)
{
 double accept,delta;
 delta=-mytemp*(scores[1]-scores[0])-ftemp*(fscores[1]-fscores[0]);
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


IMPMEMBRANE_END_NAMESPACE
