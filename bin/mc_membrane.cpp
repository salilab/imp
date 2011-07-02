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
#include <mpi.h>
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
 for(int i=0;i<nrep;++i) temp[i]=tmin*pow(tfact,i);
 return temp;
}

int* create_indexes(int nrep)
{
 int* index;
 index=new int[nrep];
 for(int i=0;i<nrep;++i) index[i]=i;
 return index;
}

int get_friend(int* index,int myrank,int step,int nrep)
{
 int myindex=index[myrank];
 int findex,frank;

 if(myindex%2==0 && step%2==0) findex=myindex+1;
 if(myindex%2==0 && step%2==1) findex=myindex-1;
 if(myindex%2==1 && step%2==0) findex=myindex-1;
 if(myindex%2==1 && step%2==1) findex=myindex+1;
 if(findex==-1)   findex=nrep-1;
 if(findex==nrep) findex=0;

 for(int i=0; i<nrep; ++i) if(index[i]==findex) frank=i;
 return frank;
}

bool get_acceptance(double score0, double score1, double T0, double T1)
{
 double accept;
 double delta=(score1-score0)*(1.0/T1-1.0/T0);
 if(delta>=0.0) accept=1.0;
 else           accept=exp(delta);
 if(rand()/RAND_MAX<=accept) return true;
 else return false;
}

int main(int argc, char* argv[])
{

int myrank, nproc;
MPI_Status  status;

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
std::ofstream myfile;
std::stringstream out;
out << myrank;
std::string names="log."+out.str();
char* name=(char*)malloc( sizeof( char ) *(names.length() +1) );;
strcpy(name, names.c_str());
myfile.open(name);

// parsing input
if(myrank==0) std::cout << "Parsing input file" << std::endl;
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
if(myrank==0) std::cout << "Creating representation" << std::endl;
core::TableRefiner* tbr=generate_TM(m,all,&mydata);

// create restraints
if(myrank==0) std::cout << "Creating restraints" << std::endl;
RestraintSet* rset=create_restraints(m,all,tbr,&mydata);

// create sampler
if(myrank==0) std::cout << "Creating sampler" << std::endl;
core::MonteCarlo* mc=setup_MonteCarlo(m,all,&mydata);

// sampling
if(myrank==0) std::cout << "Sampling" << std::endl;

// high temperature short run
mc->set_kt(mydata.MC.tmax);
mc->optimize(mydata.MC.nhot);
// reset temperature
mc->set_kt(temp[index[myrank]]);

// Monte Carlo loop
for(int imc=0;imc<mydata.MC.nsteps;++imc)
{
 mc->optimize(mydata.MC.nexc);
// time for an exchange
 int    frank=get_friend(index,myrank,imc,nproc);
 double myscore=m->evaluate(false);
 double fscore;
 int    myindex=index[myrank];
 int    findex=index[frank];
// send and receive score
 MPI_Sendrecv(&myscore,1,MPI_DOUBLE,frank,123,
              &fscore, 1,MPI_DOUBLE,frank,456, MPI_COMM_WORLD, &status);
 //MPI_Isend(&myscore, 1, MPI_DOUBLE, frank, 123, MPI_COMM_WORLD, &request);
 //MPI_Recv(&fscore,   1, MPI_DOUBLE, frank, 123, MPI_COMM_WORLD, &request2);
 bool do_accept=get_acceptance(myscore,fscore,temp[myindex],temp[findex]);
 if(do_accept){
  myindex=findex;
  myscore=fscore;
  mc->set_kt(temp[myindex]);
 }
// update index vector
 MPI_Allgather(&myindex,1,MPI_INT,index,nproc,MPI_INT,MPI_COMM_WORLD);
 myfile << imc << " " << myindex << " " << myscore << " " << mydata.MC.nexc
 << " " << mc->get_number_of_forward_steps() << "\n";
}

myfile.close();
MPI_Finalize();
return 0;
}
