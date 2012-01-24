/**
 *  \file spb_reload_minimize.cpp
 *  \brief SPB Minimizer
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/membrane.h>
#include <IMP/rmf.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

using namespace IMP;
using namespace IMP::membrane;

int main(int argc, char* argv[])
{

// parsing input
std::cout << "Parsing input file" << std::endl;
SPBParameters mydata=get_SPBParameters("config.ini","0");

// create a new model
IMP_NEW(Model,m,());

// List of particles for layer restraint
IMP_NEW(container::ListSingletonContainer,bCP_ps,(m));
IMP_NEW(container::ListSingletonContainer,CP_ps,(m));
IMP_NEW(container::ListSingletonContainer,IL2_ps,(m));
// List of Movers for MC, not used here
core::Movers mvs;
//
// PROTEIN REPRESENTATION
//
std::cout << "Creating representation" << std::endl;
atom::Hierarchies all_mol=
 create_representation(m,mydata,bCP_ps,CP_ps,IL2_ps,mvs);
atom::Hierarchies hhs;
for(unsigned int i=0;i<all_mol.size();++i){
 atom::Hierarchies hs=all_mol[i].get_children();
 for(unsigned int j=0;j<hs.size();++j) {hhs.push_back(hs[j]);}
}
//
// CREATING RESTRAINTS
//
std::cout << "Creating restraints" << std::endl;
std::map< std::string, Pointer<RestraintSet> > rst_map=
 spb_assemble_restraints(m,mydata,all_mol,bCP_ps,CP_ps,IL2_ps);
//
// PREPARE OUTPUT
//
RMF::FileHandle rh_out = RMF::create_rmf_file("traj_minimized_0.rmf");
for(unsigned int i=0;i<hhs.size();++i){rmf::add_hierarchy(rh_out, hhs[i]);}
// adding key for score
RMF::Category my_kc= rh_out.add_category("my data");
RMF::FloatKey my_key_out=rh_out.add_float_key(my_kc,"my score",true);
//
FILE *logfile;
logfile = fopen("log.emin","w");
//
// OPTIMIZER
//
IMP_NEW(core::ConjugateGradients,cg,(m));

std::cout << "Minimizing good configurations" << std::endl;
unsigned int nminimized=0;
unsigned int totframes=0;
unsigned int currentframe=0;
unsigned int iout_name=0;

// cycle on all iterations
for(unsigned iter=0;iter<mydata.niter;++iter){
 std::vector<RMF::FileHandle> rhs;
 std::vector<RMF::Category> my_kcs;
 std::vector<RMF::FloatKey> my_keys;
 std::stringstream iter_str;
 iter_str << iter;
 for(unsigned irep=0;irep<mydata.nrep;++irep){
  std::stringstream irep_str;
  irep_str << irep;
  rhs.push_back(RMF::open_rmf_file(mydata.trajfile+irep_str.str()+
                "_"+iter_str.str()+".rmf"));
  rmf::set_hierarchies(rhs[irep], hhs);
  my_kcs.push_back(rhs[irep].get_category("my data"));
  my_keys.push_back(rhs[irep].get_float_key(my_kcs[irep],"my score"));
 }
// number of frames
 unsigned int nframes=rmf::get_number_of_frames(rhs[0],hhs[0]);

//
 for(unsigned int imc=0;imc<nframes;++imc){
  for(unsigned irep=0;irep<mydata.nrep;++irep){
// increment frame counter
    ++totframes;
 // retrieve score
   double myscore = (rhs[irep].get_root_node()).get_value(my_keys[irep],imc);
// if good enough...
   if(myscore<mydata.cutoff){
// load configuration from file
    for(unsigned int i=0;i<hhs.size();++i){
     rmf::load_frame(rhs[irep],imc,hhs[i]);
    }
    double myscore_min = myscore;
    double fretr_score;
    double y2h_score;
// do coniugate gradient
    if(mydata.cg_steps>0){
     cg->do_optimize(mydata.cg_steps);
     myscore_min = m->evaluate(false);
     fretr_score = rst_map["FRET_R"]->evaluate(false);
     y2h_score   = rst_map["Y2H"]->evaluate(false);
    }
    fprintf(logfile,"%10d  %12.6f %12.6f  %12.6f %12.6f  %10d\n",
     nminimized,myscore,myscore_min,fretr_score,y2h_score,iout_name);
// write to file
    (rh_out.get_root_node()).set_value(my_key_out,myscore_min,currentframe);
    for(unsigned int i=0;i<hhs.size();++i){
     rmf::save_frame(rh_out,currentframe,hhs[i]);
    }
    ++nminimized;
    ++currentframe;
   }
// time to create a new file for output?
   if(totframes%mydata.chunk==0){
    currentframe=0;
    std::stringstream iout;
    iout_name = totframes/mydata.chunk;
    iout << iout_name;
    rh_out = RMF::create_rmf_file("traj_minimized_"+iout.str()+".rmf");
    for(unsigned int i=0;i<hhs.size();++i){rmf::add_hierarchy(rh_out, hhs[i]);}
    my_kc= rh_out.add_category("my data");
    my_key_out=rh_out.add_float_key(my_kc,"my score",true);
   }
  }
 }
}

std::cout << "Number of good configurations " << nminimized << std::endl;
fclose(logfile);

return 0;
}
