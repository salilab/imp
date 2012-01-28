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

// log file
std::stringstream out;
out << myrank;
std::string names="log"+out.str();
FILE *logfile;
logfile = fopen(names.c_str(),"w");

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
RMF::FloatKey my_key2=rh.add_float_key(my_kc,"my bias",true);

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

// wte restart
if(mydata.MC.do_wte && mydata.MC.wte_restart){
 Floats val;
 double bias;
 std::ifstream biasfile;
 std::string names="BIAS"+out.str();
 biasfile.open(names.c_str());
 while (biasfile >> bias){val.push_back(bias);}
 biasfile.close();
 Pointer<membrane::MonteCarloWithWte> ptr=
     dynamic_cast<membrane::MonteCarloWithWte*>(mc.get());
 ptr->set_bias(val);
}

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
// Monte Carlo loop
for(int imc=0;imc<mydata.MC.nsteps;++imc)
{
// run optimizer
 mc->optimize(mydata.MC.nexc);

// print statistics
 double myscore     = m->evaluate(false);
 int    myindex     = index[myrank];
 double fretr_score = rst_map["FRET_R"]->evaluate(false);
 double y2h_score   = rst_map["Y2H"]->evaluate(false);

 fprintf(logfile,"%10d %3d  %12.6f  %12.6f %12.6f  %5d %5d\n",
     imc,myindex,myscore,fretr_score,y2h_score,
     mydata.MC.nexc, mc->get_number_of_forward_steps());

// save configuration and additional information to file
 if(imc%mydata.MC.nwrite==0){
// score
  (rh.get_root_node()).set_value(my_key0,myscore,imc/mydata.MC.nwrite);
// index
  (rh.get_root_node()).set_value(my_key1,myindex,imc/mydata.MC.nwrite);
// bias
  double mybias=0.0;
  if(mydata.MC.do_wte){
   Pointer<membrane::MonteCarloWithWte> ptr=
     dynamic_cast<membrane::MonteCarloWithWte*>(mc.get());
   mybias=ptr->get_bias(myscore);
  }
  (rh.get_root_node()).set_value(my_key2,mybias,imc/mydata.MC.nwrite);
// configuration
  for(unsigned int i=0;i<all_mol.size();++i){
   atom::Hierarchies hs=all_mol[i].get_children();
   for(unsigned int j=0;j<hs.size();++j){
    rmf::save_frame(rh,imc/mydata.MC.nwrite,hs[j]);
   }
  }
 // dump bias on file if wte
  if(mydata.MC.do_wte){
   std::ofstream biasfile;
   std::string names="BIAS"+out.str();
   biasfile.open(names.c_str());
   Pointer<membrane::MonteCarloWithWte> ptr=
     dynamic_cast<membrane::MonteCarloWithWte*>(mc.get());
   double* mybias=ptr->get_bias_buffer();
   for(int i=0;i<ptr->get_nbin();++i){
    biasfile << mybias[i] << "\n";
   }
   biasfile.close();
  }
 }

// now it's time to try an exchange
 int    frank=get_friend(index,myrank,imc,nproc);
 int    findex=index[frank];
 double fscore;

// send and receive score
 MPI_Sendrecv(&myscore,1,MPI_DOUBLE,frank,myrank,
               &fscore,1,MPI_DOUBLE,frank,frank,
               MPI_COMM_WORLD, &status);

// if WTE, calculate U_mybias(myscore) and U_mybias(fscore) and exchange
 double delta_wte=0.0;

 if(mydata.MC.do_wte){
  Pointer<membrane::MonteCarloWithWte> ptr=
   dynamic_cast<membrane::MonteCarloWithWte*>(mc.get());
  double U_mybias[2]={ptr->get_bias(myscore),ptr->get_bias(fscore)};
  double U_fbias[2];
  MPI_Sendrecv(U_mybias,2,MPI_DOUBLE,frank,myrank,
                U_fbias,2,MPI_DOUBLE,frank,frank,
                MPI_COMM_WORLD, &status);
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
   Pointer<membrane::MonteCarloWithWte> ptr=
    dynamic_cast<membrane::MonteCarloWithWte*>(mc.get());
   ptr->set_w0(mydata.MC.wte_w0*temp[myindex]/mydata.MC.tmin);
   int     nbins=ptr->get_nbin();
   double* mybias=ptr->get_bias_buffer();
   double* fbias=new double[nbins];
   MPI_Sendrecv(mybias,nbins,MPI_DOUBLE,frank,myrank,
                 fbias,nbins,MPI_DOUBLE,frank,frank,
                MPI_COMM_WORLD, &status);
   Floats val(fbias, fbias+nbins);
   ptr->set_bias(val);
   delete [] fbias;
  }
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
MPI_Barrier(MPI_COMM_WORLD);
// finalize MPI
MPI_Finalize();
return 0;
}
