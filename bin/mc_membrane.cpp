/**
 *  \file mc_membrane.cpp
 *  \brief Membrane MonteCarlo
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/membrane.h>
#include <IMP/rmf.h>
#include <mpi.h>
#include <boost/scoped_array.hpp>
#include <time.h>
#include <fstream>
#include <sstream>

using namespace IMP;
using namespace IMP::membrane;

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

int main(int argc, char* argv[])
{

int myrank,nproc;
MPI_Status  status;
MPI_Request request;
MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD,&nproc);
MPI_Comm_rank(MPI_COMM_WORLD,&myrank);

// initialize seed
unsigned int iseed = time(NULL);
// broadcast seed
MPI_Bcast(&iseed, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
// initialize random generator
srand (iseed);

// log file
std::ofstream logfile;
std::stringstream out;
out << myrank;
std::string names="log"+out.str();
char* name=(char*)malloc( sizeof( char ) *(names.length() +1) );;
strcpy(name, names.c_str());
logfile.open(name);

// parsing input
if(myrank==0) {std::cout << "Parsing input file" << std::endl;}
Parameters mydata=get_parameters("config.ini");

// create temperature and index array
double* temp=create_temperatures(mydata.MC.tmin,mydata.MC.tmax,nproc);
int*    index=create_indexes(nproc);

// create a new model
IMP_NEW(Model,m,());
// root hierarchy
IMP_NEW(Particle,ph,(m));
atom::Hierarchy all=atom::Hierarchy::setup_particle(ph);

// create representation
if(myrank==0) {std::cout << "Creating representation" << std::endl;}
core::TableRefiner* tbr=generate_TM(m,all,&mydata);

// trajectory file
std::string trajname="traj"+out.str()+".rmf";
rmf::RootHandle rh = rmf::RootHandle(trajname,true);
atom::HierarchiesTemp hs=all.get_children();
for(int i=0;i<hs.size();++i) {rmf::add_hierarchy(rh, hs[i]);}

// create restraints
if(myrank==0) {std::cout << "Creating restraints" << std::endl;}
RestraintSet* rset=create_restraints(m,all,tbr,&mydata);

// create sampler
if(myrank==0) {std::cout << "Creating sampler" << std::endl;}
core::MonteCarlo* mc=setup_MonteCarlo(m,all,temp[index[myrank]],&mydata);

// sampling
if(myrank==0) {std::cout << "Sampling" << std::endl;}

// high temperature short run
mc->set_kt(mydata.MC.tmax);
mc->optimize(mydata.MC.nhot);
// reset temperature
mc->set_kt(temp[index[myrank]]);

// Monte Carlo loop
for(int imc=0;imc<mydata.MC.nsteps;++imc)
{
// run optimizer
 mc->optimize(mydata.MC.nexc);

// print statistics
 double myscore=m->evaluate(false);
 int    myindex=index[myrank];
 logfile << imc << " " << myindex << " " << myscore << " "
 << mydata.MC.nexc << " " << mc->get_number_of_forward_steps() << "\n";

// now it's time to try an exchange
 int    frank=get_friend(index,myrank,imc,nproc);
 int    findex=index[frank];
 double fscore;
// send and receive score
 MPI_Isend(&myscore, 1, MPI_DOUBLE, frank, 123, MPI_COMM_WORLD, &request);
 MPI_Recv(&fscore,   1, MPI_DOUBLE, frank, 123, MPI_COMM_WORLD, &status);

// if WTE, calculate U_mybias(myscore) and U_mybias(fscore) and exchange
 double delta_wte=0.0;
 if(mydata.MC.do_wte){
  membrane::MonteCarloWithWte *ptr=
   dynamic_cast<membrane::MonteCarloWithWte*>(mc);
  double U_mybias[2]={ptr->get_bias(myscore),ptr->get_bias(fscore)};
  double U_fbias[2];
  MPI_Isend(U_mybias, 2, MPI_DOUBLE, frank, 123, MPI_COMM_WORLD, &request);
  MPI_Recv(U_fbias,   2, MPI_DOUBLE, frank, 123, MPI_COMM_WORLD, &status);
  delta_wte=(U_mybias[0]-U_mybias[1])/temp[myindex]+
            (U_fbias[0] -U_fbias[1])/ temp[findex];
 }
// calculate acceptance
 bool do_accept=get_acceptance(myscore,fscore,delta_wte,
                               temp[myindex],temp[findex]);
// if accepted exchange what is needed
 if(do_accept){
  myindex=findex;
  mc->set_kt(temp[myindex]);
// if WTE, rescale W0 and exchange bias
  if(mydata.MC.do_wte){
   membrane::MonteCarloWithWte *ptr=
    dynamic_cast<membrane::MonteCarloWithWte*>(mc);
   ptr->set_w0(mydata.MC.wte_w0*temp[myindex]/mydata.MC.tmin);
   int     nbins=ptr->get_nbin();
   double* mybias=ptr->get_bias_buffer();
   double* fbias=new double[2*nbins];
   MPI_Isend(mybias, 2*nbins, MPI_DOUBLE, frank, 123, MPI_COMM_WORLD, &request);
   MPI_Recv(fbias,   2*nbins, MPI_DOUBLE, frank, 123, MPI_COMM_WORLD, &status);
   Floats val(fbias, fbias+2*nbins);
   ptr->set_bias(val);
   delete fbias;
  }
 }

// in any case, update index vector
 int buf[nproc];
 for(int i=0; i<nproc; ++i) {buf[i]=0;}
 buf[myrank]=myindex;
 MPI_Allreduce(buf,index,nproc,MPI_INT,MPI_SUM,MPI_COMM_WORLD);

// save configuration to file
 if(imc%mydata.MC.nwrite==0){
  for(int i=0;i<hs.size();++i){
   rmf::save_frame(rh,imc/mydata.MC.nwrite,hs[i]);
  }
 }
}

logfile.close();
MPI_Finalize();
return 0;
}
