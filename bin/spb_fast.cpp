/**
 *  \file spb.cpp
 *  \brief SPB MonteCarlo
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/membrane.h>
#include <IMP/rmf.h>
#include "mpi.h"
#include <boost/scoped_array.hpp>
#include <time.h>
#include <fstream>
#include <sstream>
#include <stdio.h>

using namespace IMP;
using namespace IMP::membrane;

int main(int argc, char* argv[])
{

MPI_Init(&argc,&argv);
int nproc, myrank;
MPI_Comm_size(MPI_COMM_WORLD, &nproc);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
MPI_Status status;

// initialize seed
unsigned int iseed = time(NULL);
// broadcast seed
MPI_Bcast(&iseed,1,MPI_UNSIGNED,0,MPI_COMM_WORLD);
// initialize random generator
srand (iseed);

// log and fret file
std::stringstream out;
out << myrank;
std::string logname="log"+out.str();
FILE *logfile;
logfile = fopen(logname.c_str(),"w");
std::string fretname="log.fret"+out.str();
FILE *fretfile;
fretfile = fopen(fretname.c_str(),"w");

std::string inputfile="config.ini";
int i=1;
while(i<argc){ if(strcmp(argv[i],"-in")==0){++i; inputfile=argv[i];}
 ++i;
}

// parsing input
if(myrank==0) {std::cout << "Parsing input file" << std::endl;}
SPBParameters mydata=get_SPBParameters(inputfile,out.str());

// create temperature and index array
double* temp=create_temperatures(mydata.MC.tmin,mydata.MC.tmax,nproc);
int*    index=create_indexes(nproc);

// create a new model
IMP_NEW(Model,m,());

// List of particles for layer restraint
IMP_NEW(container::ListSingletonContainer,bCP_ps,(m));
IMP_NEW(container::ListSingletonContainer,CP_ps,(m));
IMP_NEW(container::ListSingletonContainer,IL2_ps,(m));
// List of Movers for MC
core::Movers mvs;
//
// PROTEIN REPRESENTATION
//
if(myrank==0) {std::cout << "Creating representation" << std::endl;}
atom::Hierarchies all_mol=
 create_representation(m,mydata,bCP_ps,CP_ps,IL2_ps,mvs);
//
// restart from individual rmf file
if(mydata.file_list.size()>0){
 if(myrank==0){std::cout << "Restart from file" << std::endl;}
 load_restart(all_mol,mydata);
}
//
// Prepare output file
std::string trajname="traj"+out.str()+".rmf";
RMF::FileHandle rh = RMF::create_rmf_file(trajname);

for(unsigned int i=0;i<all_mol.size();++i){
 atom::Hierarchies hs=all_mol[i].get_children();
 for(unsigned int j=0;j<hs.size();++j) {rmf::add_hierarchy(rh, hs[j]);}
}
// adding key for score and index
RMF::Category my_kc= rh.add_category("my data");
RMF::FloatKey my_key0=rh.add_float_key(my_kc,"my score",true);
RMF::IntKey   my_key1=rh.add_int_key(my_kc,"my index",true);

//
// CREATING RESTRAINTS
//
if(myrank==0) {std::cout << "Creating restraints" << std::endl;}
std::map< std::string, Pointer<RestraintSet> > rst_map=
 spb_assemble_restraints(m,mydata,all_mol,bCP_ps,CP_ps,IL2_ps);

//
if(myrank==0) {std::cout << "Setup sampler" << std::endl;}
Pointer<core::MonteCarlo> mc=
 setup_SPBMonteCarlo(m,mvs,temp[index[myrank]],mydata);

// hot steps
if(mydata.MC.nhot>0){
 if(myrank==0) {std::cout << "High temperature initialization" << std::endl;}
 mc->set_kt(mydata.MC.tmax);
 mc->optimize(mydata.MC.nhot);
}
// set temperature
mc->set_kt(temp[index[myrank]]);

// sampling
if(myrank==0) {std::cout << "Sampling" << std::endl;}

// reset counter of good models
unsigned ngood=0;

// Monte Carlo loop
for(int imc=0;imc<mydata.MC.nsteps;++imc)
{
// run optimizer
 mc->optimize(mydata.MC.nexc);

// total score and index
 double myscore  = m->evaluate(false);
 int    myindex  = index[myrank];

// get individual contribution to fret score
 Floats fretr_scores;
 bool fretisgood=true;
 for(unsigned i=0;i<rst_map["FRET_R"]->get_number_of_restraints();++i){
  double score = rst_map["FRET_R"]->get_restraint(i)->evaluate(false);
  score = sqrt(2.0*score) / mydata.Fret.sigmas[i];
  if(score > mydata.Fret.sigmamult){fretisgood=false;}
  fretr_scores.push_back(score);
 }

// is this a good configuration?
 if((mydata.Fret.sigmafilter && fretisgood) || !mydata.Fret.sigmafilter){
// print main log file
  double fretr_score = rst_map["FRET_R"]->evaluate(false);
  double y2h_score   = rst_map["Y2H"]->evaluate(false);
  fprintf(logfile,"%10d %10d  %3d  %12.6f  %12.6f %12.6f\n",
     ngood,imc,myindex,myscore,fretr_score,y2h_score);

// print fret log file
  fprintf(fretfile,"%10d  ",ngood);
  for(unsigned i=0;i<fretr_scores.size();++i){
   fprintf(fretfile,"%12.6lf ",fretr_scores[i]);
  }
  fprintf(fretfile,"\n");

// save configuration and additional information to file
// score
  (rh.get_root_node()).set_value(my_key0,myscore,ngood);
// index
  (rh.get_root_node()).set_value(my_key1,myindex,ngood);
// configuration
  for(unsigned int i=0;i<all_mol.size();++i){
   atom::Hierarchies hs=all_mol[i].get_children();
   for(unsigned int j=0;j<hs.size();++j){
    rmf::save_frame(rh,ngood,hs[j]);
   }
  }
// increment counter of good models
  ++ngood;
 }

// now it's time to try an exchange
 int    frank=get_friend(index,myrank,imc,nproc);
 int    findex=index[frank];
 double fscore;

// send and receive score
 MPI_Sendrecv(&myscore,1,MPI_DOUBLE,frank,myrank,
               &fscore,1,MPI_DOUBLE,frank,frank,
               MPI_COMM_WORLD, &status);

 // calculate acceptance
 bool do_accept=get_acceptance(myscore,fscore,0.0,
                               temp[myindex],temp[findex]);

// if accepted exchange what is needed
 if(do_accept){
  myindex=findex;
  mc->set_kt(temp[myindex]);
 }

// in any case, update index vector
 MPI_Barrier(MPI_COMM_WORLD);
 int sbuf[nproc],rbuf[nproc];
 for(int i=0;i<nproc;++i){sbuf[i]=0;}
 sbuf[myrank]=myindex;
 MPI_Allreduce(sbuf,rbuf,nproc,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
 for(int i=0;i<nproc;++i){index[i]=rbuf[i];}
}

MPI_Barrier(MPI_COMM_WORLD);
// close rmf
rh.flush();
rh=RMF::FileHandle();
// flush and close logfile
fflush(logfile);
fclose(logfile);
// flush and close fretfile
fflush(fretfile);
fclose(fretfile);
MPI_Barrier(MPI_COMM_WORLD);
// finalize MPI
MPI_Finalize();
return 0;
}
