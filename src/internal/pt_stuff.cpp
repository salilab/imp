/**
 *  \file  pt_stuff.cpp
 *  \brief Useful stuff for parallel tempering
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane.h>
#include <math.h>

IMPMEMBRANE_BEGIN_NAMESPACE

double* create_temperatures(double tmin,double tmax,int nrep)
{
 double* temp;
 temp=new double[nrep];
 double tfact=exp(log(tmax/tmin)/double(nrep-1));
 for(int i=0;i<nrep;++i) {temp[i]=tmin*pow(tfact,i);}
 return temp;
}

int* create_indexes(int nrep)
{
 int* index;
 index=new int[nrep];
 for(int i=0;i<nrep;++i) {index[i]=i;}
 return index;
}

int get_friend(int* index,int myrank,int step,int nrep)
{
 int myindex=index[myrank];
 int findex=0;
 int frank=0;

 if(myindex%2==0 && step%2==0) {findex=myindex+1;}
 if(myindex%2==0 && step%2==1) {findex=myindex-1;}
 if(myindex%2==1 && step%2==0) {findex=myindex-1;}
 if(myindex%2==1 && step%2==1) {findex=myindex+1;}
 if(findex==-1)   {findex=nrep-1;}
 if(findex==nrep) {findex=0;}

 for(int i=0; i<nrep; ++i) {if(index[i]==findex) {frank=i;}}
 return frank;
}

bool get_acceptance(double score0,double score1,double delta_wte,
                    double T0,double T1)
{
 double accept,delta;
 delta=(score1-score0)*(1.0/T1-1.0/T0)+delta_wte;
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
