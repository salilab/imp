/**
 *  \file mc_spb.cpp
 *  \brief SPB MonteCarlo
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/membrane.h>
#include <IMP/rmf.h>
#include <IMP/display.h>
#include <boost/scoped_array.hpp>
#include <time.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "mpi.h"

using namespace IMP;
using namespace IMP::membrane;

int main(int argc, char* argv[])
{

MPI::Init(argc,argv);
const int nproc= MPI::COMM_WORLD.Get_size();
const int myrank=MPI::COMM_WORLD.Get_rank();

// initialize seed
unsigned int iseed = time(NULL);
// broadcast seed
MPI::COMM_WORLD.Bcast(&iseed,1,MPI::UNSIGNED,0);
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
SPBParameters mydata=get_SPBParameters("config.ini");

// create temperature and index array
double* temp=create_temperatures(mydata.MC.tmin,mydata.MC.tmax,nproc);
int*    index=create_indexes(nproc);

// create a new model
IMP_NEW(Model,m,());
//
// PROTEIN REPRESENTATION
//
if(myrank==0) {std::cout << "Creating representation" << std::endl;}
// List of particles for layer restraint
IMP_NEW(container::ListSingletonContainer,bCP_ps,(m));
IMP_NEW(container::ListSingletonContainer,CP_ps,(m));
IMP_NEW(container::ListSingletonContainer,IL2_ps,(m));
// List of Movers for MC
core::Movers mvs;
atom::Hierarchies all_mol=
 create_representation(m,mydata,bCP_ps,CP_ps,IL2_ps,mvs);
//
// RESTART from individual rmf file
if(mydata.file_list.size()>0){
 if(myrank==0){std::cout << "Restart from file" << std::endl;}
 load_restart(all_mol,mydata);
}
//
// Prepare output file
std::string trajname="traj"+out.str()+".rmf";
rmf::RootHandle rh = rmf::create_rmf_file(trajname);
for(unsigned int i=0;i<all_mol.size();++i){
 atom::Hierarchies hs=all_mol[i].get_children();
 for(unsigned int j=0;j<hs.size();++j) {rmf::add_hierarchy(rh, hs[j]);}
}
// adding key for score
rmf::Category my_kc= rh.add_category("my data");
rmf::FloatKey my_key=rh.add_float_key(my_kc,"my score",true);

//
// CREATING RESTRAINTS
if(myrank==0) {std::cout << "Creating restraints" << std::endl;}
//
// Excluded volume
//
add_SPBexcluded_volume(m,all_mol,mydata.kappa);
//
// Symmetry
//
add_symmetry_restraint(m,all_mol,mydata.trs);
//
// Layer restraint
//
// CP and below
add_layer_restraint(m, bCP_ps,
 FloatRange(-1.0e+34,mydata.CP_thickness/2.0),mydata.kappa);
// inside CP
add_layer_restraint(m, CP_ps,
 FloatRange(-mydata.CP_thickness/2.0,mydata.CP_thickness/2.0),
 mydata.kappa);
// inside IL2
double dz=mydata.IL2_centers[0][2];
add_layer_restraint(m, IL2_ps,
 FloatRange(-mydata.IL2_thickness/2.0+dz,mydata.IL2_thickness/2.0+dz),
 mydata.kappa);
//
// TILT restraint
//
if(mydata.add_tilt && mydata.protein_list["Spc110p"]){
 add_tilt(m,all_mol[0],"Spc110p",mydata.tilt,mydata.kappa);
}
//
// FRET
//
if(mydata.add_fret){
// intra-CP
 add_fret_restraint(m,all_mol[0], "Spc29p",   "C",
                      all_mol,     "Cmd1p",   "C", 1.69,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0], "Spc29p",   "N",
                      all_mol,     "Cmd1p",   "C", 1.75,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0],  "Spc29p",  "C",
                      all_mol,    "Spc110p",  "C", 1.37,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0], "Spc29p",   "C",
                      all_mol,    "Spc42p",   "N", 2.05,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0],  "Cmd1p",   "C",
                      all_mol,    "Spc42p",   "N", 2.07,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0],   "Cmd1p",  "C",
                      all_mol,    "Spc110p",  "C", 2.15,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0],  "Spc42p",  "N",
                      all_mol,    "Spc110p",  "C", 2.02,
                      mydata.kappa, mydata.add_GFP);
// inter CP-IL2

 add_fret_restraint(m,all_mol[0],  "Spc42p",  "C",
                      all_mol,    "Spc110p",  "C", 1.07,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0], "Cnm67p_c", "C",
                      all_mol,       "Cmd1p", "C", 1.09,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0], "Spc42p",   "C",
                      all_mol,     "Cmd1p",   "C", 1.1,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0], "Cnm67p_c", "C",
                      all_mol,      "Spc29p", "C", 1.1,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0], "Cnm67p_c", "C",
                      all_mol,      "Spc42p", "N", 1.13,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0], "Spc42p",   "C",
                      all_mol,    "Spc29p",   "C", 1.17,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0], "Spc42p",   "C",
                      all_mol,    "Spc42p",   "N", 1.27,
                      mydata.kappa, mydata.add_GFP);

// intra-IL2
 add_fret_restraint(m,all_mol[0],   "Spc42p", "C",
                      all_mol,    "Cnm67p_c", "C", 2.29,
                      mydata.kappa, mydata.add_GFP);
}
if(mydata.add_y2h){
//
// TWO-HYBRID SCREENING
//
// CP
 // substitued by link restraint
 //add_y2h_restraint(m,all_mol, "Cmd1p",      "ALL",
 //                    all_mol, "Spc110p",    IntRange(900,927), mydata.kappa);
 add_y2h_restraint(m,all_mol[0],  "Spc42p",               "N",
                     all_mol,    "Spc110p",               "C", mydata.kappa);
 add_y2h_restraint(m,all_mol[0],  "Spc29p",             "ALL",
                     all_mol,    "Spc110p", IntRange(811,944), mydata.kappa);
 add_y2h_restraint(m,all_mol[0], "Spc110p",               "C",
                     all_mol,    "Spc110p",               "C", mydata.kappa);
 add_y2h_restraint(m,all_mol[0],  "Spc42p",   IntRange(1,138),
                     all_mol,     "Spc29p",             "ALL", mydata.kappa);
// Having a rigid coiled-coil, this is always satisfied
// add_y2h_restraint(m,all_mol[0], "Spc42p",     IntRange(1,138),
//                     all_mol, "Spc42p",     IntRange(1,138), mydata.kappa);
// IL2
 add_y2h_restraint(m,all_mol[0], "Cnm67p_c",             "C",
                     all_mol,      "Spc42p",             "C", mydata.kappa);
}
//
// Add link between Spc110p_C and Cmd1p
  add_link(m,all_mol[0],   "Cmd1p",             "ALL",
             all_mol,    "Spc110p", IntRange(900,927),mydata.kappa);
// Add link with GFPs
if(mydata.add_GFP){
  add_link(m,all_mol[0],      "Spc110p", "C",
             all_mol,   "Spc110p-C-GFP", "N", mydata.kappa);
  add_link(m,all_mol[0],        "Cmd1p", "N",
             all_mol,     "Cmd1p-N-GFP", "C", mydata.kappa);
  add_link(m,all_mol[0],        "Cmd1p", "C",
             all_mol,     "Cmd1p-C-GFP", "N", mydata.kappa);
  add_link(m,all_mol[0],       "Spc42p", "N",
             all_mol,    "Spc42p-N-GFP", "C", mydata.kappa);
  add_link(m,all_mol[0],       "Spc42p", "C",
             all_mol,    "Spc42p-C-GFP", "N", mydata.kappa);
  add_link(m,all_mol[0],       "Spc29p", "N",
             all_mol,    "Spc29p-N-GFP", "C", mydata.kappa);
  add_link(m,all_mol[0],       "Spc29p", "C",
             all_mol,    "Spc29p-C-GFP", "N", mydata.kappa);
  add_link(m,all_mol[0],     "Cnm67p_c", "C",
             all_mol,  "Cnm67p_c-C-GFP", "N", mydata.kappa);
}

//
if(myrank==0) {std::cout << "Setup sampler" << std::endl;}
Pointer<core::MonteCarlo> mc=
 setup_SPBMonteCarlo(m,mvs,temp[index[myrank]],mydata);
//mc->set_use_incremental_evaluate(true);

// hot steps
if(mydata.MC.nhot>0){
 if(myrank==0) {std::cout << "High temperature initialization" << std::endl;}
 mc->set_kt(mydata.MC.tmax);
 mc->optimize(mydata.MC.nhot);
}
// set temperature
mc->set_kt(temp[index[myrank]]);

// GO!
if(myrank==0) {std::cout << "Sampling" << std::endl;}
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

// save configuration and score to file
 if(imc%mydata.MC.nwrite==0){
  rh.set_value(my_key,myscore,imc/mydata.MC.nwrite);
  for(unsigned int i=0;i<all_mol.size();++i){
   atom::Hierarchies hs=all_mol[i].get_children();
   for(unsigned int j=0;j<hs.size();++j){
    rmf::save_frame(rh,imc/mydata.MC.nwrite,hs[j]);
   }
  }
 }
// now it's time to try an exchange
 if(nproc>1){
  int    frank=get_friend(index,myrank,imc,nproc);
  int    findex=index[frank];
  double fscore;
// send and receive score
  MPI::COMM_WORLD.Isend(&myscore,1,MPI::DOUBLE,frank,123);
  MPI::COMM_WORLD.Recv (&fscore, 1,MPI::DOUBLE,frank,123);

 // if WTE, calculate U_mybias(myscore) and U_mybias(fscore) and exchange
  double delta_wte=0.0;

  if(mydata.MC.do_wte){
   Pointer<membrane::MonteCarloWithWte> ptr=
    dynamic_cast<membrane::MonteCarloWithWte*>(mc.get());
   double U_mybias[2]={ptr->get_bias(myscore),ptr->get_bias(fscore)};
   double U_fbias[2];
   MPI::COMM_WORLD.Isend(U_mybias,2,MPI::DOUBLE,frank,123);
   MPI::COMM_WORLD.Recv (U_fbias, 2,MPI::DOUBLE,frank,123);
   delta_wte=(U_mybias[0]-U_mybias[1])/temp[myindex]+
             (U_fbias[0] -U_fbias[1])/ temp[findex];
  }

 // calculate acceptance
  bool do_accept=
   get_acceptance(myscore,fscore,delta_wte,temp[myindex],temp[findex]);

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
    double* fbias=new double[2*nbins];
    MPI::COMM_WORLD.Isend(mybias,2*nbins,MPI::DOUBLE,frank,123);
    MPI::COMM_WORLD.Recv (fbias, 2*nbins,MPI::DOUBLE,frank,123);
    Floats val(fbias, fbias+2*nbins);
    ptr->set_bias(val);
   }
  }

 // in any case, update index vector
  int buf[nproc];
  for(int i=0; i<nproc; ++i) {buf[i]=0;}
  buf[myrank]=myindex;
  MPI::COMM_WORLD.Allreduce(buf,index,nproc,MPI::INT,MPI::SUM);
 } // end if nproc > 1
} // end mc

MPI::COMM_WORLD.Barrier();
logfile.flush();
logfile.close();
MPI::Finalize();
return 0;
}
