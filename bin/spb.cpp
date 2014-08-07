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
#include <IMP/isd2.h>
#include <IMP/rmf.h>
#include "mpi.h"
#include <boost/scoped_array.hpp>
#include <time.h>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <map>
#include <string>

using namespace IMP;
using namespace IMP::membrane;

int main(int argc, char* argv[])
{

MPI_Init(&argc,&argv);
int nproc, myrank;
MPI_Comm_size(MPI_COMM_WORLD, &nproc);
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
MPI_Status status;

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
IMP_NEW(container::ListSingletonContainer,CP_ps,(m));
IMP_NEW(container::ListSingletonContainer,IL2_ps,(m));
// List of Movers for MC
core::Movers mvs;

//
// ISD PARTICLES
//
std::map<std::string, Pointer<Particle> > ISD_ps=
 add_ISD_particles(m,mydata,mvs);
// create list of particles from map
Particles ISD_ps_list;
std::map<std::string, Pointer<Particle> >::iterator itr;
for(itr = ISD_ps.begin(); itr != ISD_ps.end(); ++itr){
 ISD_ps_list.push_back((*itr).second);
}
//
// PROTEIN REPRESENTATION
//
if(myrank==0) {std::cout << "Creating representation" << std::endl;}
atom::Hierarchies all_mol=
 create_representation(m,mydata,CP_ps,IL2_ps,mvs,
                       ISD_ps["SideXY"],ISD_ps["SideZ"],myrank);

// re-initialize seed
unsigned int iseed = time(NULL);
// broadcast seed
MPI_Bcast(&iseed,1,MPI_UNSIGNED,0,MPI_COMM_WORLD);
// initialize random generator
srand (iseed);

//
// Add cell Mover
//
Particles ps0=atom::get_leaves(all_mol[0]);
IMP_NEW(membrane::CellMover,cm,(ISD_ps["SideXY"],ps0,mydata.MC.dSide));
mvs.push_back(cm);
//
// restart from individual rmf file
//
if(mydata.file_list.size()>0){
 if(myrank==0){std::cout << "Restart coordinates from file" << std::endl;}
 load_restart(all_mol,mydata);
}
//
// reread ISD particles
//
if(mydata.isd_restart){
 if(myrank==0){std::cout << "Restart ISD particles from file" << std::endl;}
 RMF::FileHandle rh = RMF::open_rmf_file(mydata.isd_restart_file);
 rmf::link_particles(rh, ISD_ps_list);
 unsigned int iframe=rh.get_number_of_frames();
 rmf::load_frame(rh,iframe-1);
}
//
// Prepare output file for coordinates
//
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
// Prepare output file for ISD particles
//
std::string isdname="trajisd"+out.str()+".rmf";
RMF::FileHandle rh_isd = RMF::create_rmf_file(isdname);
rmf::add_particles(rh_isd, ISD_ps_list);

//
// CREATING RESTRAINTS
//
if(myrank==0) {std::cout << "Creating restraints" << std::endl;}
std::map< std::string, Pointer<RestraintSet> > rst_map=
 spb_assemble_restraints(m,mydata,all_mol,CP_ps,IL2_ps,ISD_ps);

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
 if(biasfile.is_open()){
  while (biasfile >> bias){val.push_back(bias);}
  Pointer<membrane::MonteCarloWithWte> ptr=
     dynamic_cast<membrane::MonteCarloWithWte*>(mc.get());
  ptr->set_bias(val);
  biasfile.close();
 }
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

// get score and index
 double myscore = m->evaluate(false);
 int    myindex = index[myrank];
// and bias
 double mybias = 0.;
 if(mydata.MC.do_wte){
   Pointer<membrane::MonteCarloWithWte> ptr=
     dynamic_cast<membrane::MonteCarloWithWte*>(mc.get());
   mybias=ptr->get_bias(myscore);
 }

// print log file, save configuration and additional information to file
 if(imc%mydata.MC.nwrite==0){

// print statistics
  double fretr_score=0.0;
  double y2h_score=0.0;

  if(mydata.add_fret){fretr_score=rst_map["FRET_R"]->evaluate(false);}
  if(mydata.add_y2h) {y2h_score=rst_map["Y2H"]->evaluate(false);}

  // print stuff
  fprintf(logfile,
         "TimeStep %10d REM_Index %3d Frame %10d\n",
         imc,myindex,imc/mydata.MC.nwrite);
  fprintf(logfile,
         "TimeStep %10d Total %12.6f Fret %12.6f Y2h %12.6f Bias %12.6f\n",
         imc,myscore,fretr_score,y2h_score,mybias);
  fprintf(logfile,
         "TimeStep %10d Temperature %12.6f Acceptance %12.6f\n",
         imc,mc->get_kt(),
         float(mc->get_number_of_forward_steps())/float(mydata.MC.nexc));
  if(mydata.add_fret){
   fprintf(logfile,"TimeStep %10d Kda %12.6f Ida %12.6f Sigma0 %12.6f\n",
           imc,
           isd2::Scale(ISD_ps["Kda"]).get_scale(),
           isd2::Scale(ISD_ps["Ida"]).get_scale(),
           isd2::Scale(ISD_ps["Sigma0"]).get_scale());
   fprintf(logfile,"TimeStep %10d R0 %12.6f pBl %12.6f\n",
           imc,
           isd2::Scale(ISD_ps["R0"]).get_scale(),
           isd2::Scale(ISD_ps["pBl"]).get_scale());
  }
  //if(mydata.add_new_fret){
  // fprintf(logfile,"TimeStep %10d Kda_new %12.6f Ida_new %12.6f\n",
  //        imc,
  //        isd2::Scale(ISD_ps["Kda_new"]).get_scale(),
  //        isd2::Scale(ISD_ps["Ida_new"]).get_scale());
  //}
  fprintf(logfile,"TimeStep %10d CP %12.6f GAP %12.6f Cell %12.6f\n",
          imc,
          isd2::Scale(ISD_ps["CP_B"]).get_scale()-
          isd2::Scale(ISD_ps["CP_A"]).get_scale(),
          isd2::Scale(ISD_ps["GAP_A"]).get_scale(),
          mydata.sideMin*isd2::Scale(ISD_ps["SideXY"]).get_scale());
  // print fmod, fmod_err, ferr, for every data point
  if(mydata.add_fret){
   for(unsigned i=0;i<rst_map["FRET_R"]->get_number_of_restraints();++i){
    Pointer<isd2::FretRestraint> rst=
     dynamic_cast<isd2::FretRestraint*>(rst_map["FRET_R"]->get_restraint(i));
    std::string name = rst->get_name();
    Float fmod       = rst->get_model_fretr();
    Float fmod_err   = rst->get_standard_error();
    Float fexp       = rst->get_experimental_value();
    fprintf(logfile,
         "TimeStep %10d Name %30s  Model %6.3f  Model_Error %6.3f  Exp %6.3f\n",
         imc,name.c_str(),fmod,fmod_err,fexp);
   }
  }

// save score to rmf
  (rh.get_root_node()).set_value(my_key0,myscore,imc/mydata.MC.nwrite);
// save index to rmf
  (rh.get_root_node()).set_value(my_key1,myindex,imc/mydata.MC.nwrite);
// save bias to rmf
  (rh.get_root_node()).set_value(my_key2,mybias,imc/mydata.MC.nwrite);
// save configuration to rmf
  rmf::save_frame(rh,imc/mydata.MC.nwrite);

// save ISD particles
  rmf::save_frame(rh_isd,imc/mydata.MC.nwrite);

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
 } // end printout

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
rh_isd.flush();
rh_isd=RMF::FileHandle();
MPI_Barrier(MPI_COMM_WORLD);
// flush and close logfile
fflush(logfile);
fclose(logfile);
MPI_Barrier(MPI_COMM_WORLD);
// finalize MPI
MPI_Finalize();
return 0;
}
