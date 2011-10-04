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
#include <IMP/display.h>
#include <IMP/statistics.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <RMF/RootHandle.h>

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
spb_assemble_restraints(m,mydata,all_mol,bCP_ps,CP_ps,IL2_ps);
//
// PREPARE OUTPUT
//
RMF::RootHandle rh_out = RMF::create_rmf_file("traj_minimized.rmf");
for(unsigned int i=0;i<hhs.size();++i){rmf::add_hierarchy(rh_out, hhs[i]);}
// adding key for score
RMF::Category my_kc= rh_out.add_category("my data");
RMF::FloatKey my_key_out=rh_out.add_float_key(my_kc,"my score",true);
//
std::ofstream logfile;
logfile.open("log.emin");
//
// OPTIMIZER
//
IMP_NEW(core::ConjugateGradients,cg,(m));
//
// CLUSTERING
//
//IMP_NEW(ConfigurationSet,cset,(m));

std::cout << "Minimizing good configurations" << std::endl;
std::vector<unsigned> counter;
unsigned int nminimized=0;

// cycle on all iterations
for(unsigned iter=0;iter<mydata.niter;++iter){
 std::vector<RMF::RootHandle> rhs;
 std::stringstream iter_str;
 iter_str << iter;
 for(unsigned irep=0;irep<mydata.nrep;++irep){
  std::stringstream irep_str;
  irep_str << irep;
  rhs.push_back(RMF::open_rmf_file(mydata.trajfile+irep_str.str()+
                                   "_"+iter_str.str()+".rmf"));
  rmf::set_hierarchies(rhs[irep], hhs);
 }
 // getting key for score
 RMF::Category my_kc= rhs[0].add_category("my data");
 RMF::FloatKey my_key=rhs[0].get_float_key(my_kc,"my score");
// number of frames
 unsigned int nframes=rmf::get_number_of_frames(rhs[0],hhs[0]);

//
 for(unsigned int imc=0;imc<nframes;++imc){
  for(unsigned irep=0;irep<mydata.nrep;++irep){
 // retrieve score
   double myscore = rhs[irep].get_value(my_key,imc);
// if good enough...
   if(myscore<mydata.cutoff){
// load configuration from file
    for(unsigned int i=0;i<hhs.size();++i){
     rmf::load_frame(rhs[irep],imc,hhs[i]);
    }
// do coniugate gradient
    if(mydata.cg_steps>0){cg->do_optimize(mydata.cg_steps);}
    double myscore_min = m->evaluate(false);
    logfile << nminimized << " " << imc << " " <<
     myscore << " " << myscore_min << "\n";
// write to file
    rh_out.set_value(my_key_out,myscore_min,nminimized);
    for(unsigned int i=0;i<hhs.size();++i){
     rmf::save_frame(rh_out,nminimized,hhs[i]);
    }
    ++nminimized;
    //cset->save_configuration();
   }
  }
  if(imc%mydata.chunk==0){counter.push_back(nminimized);}
 }
}
std::cout << "Number of good configurations " << nminimized << std::endl;

/*
std::cout << "Clustering" << std::endl;

IMP_NEW(container::ListSingletonContainer,lsc,(m));
for(unsigned int i=0;i<hhs.size();++i){
 lsc->add_particles(atom::get_leaves(hhs[i]));
}
IMP_NEW(statistics::ConfigurationSetXYZEmbedding,emb,(cset,lsc));

statistics::PartitionalClusteringWithCenter *clustering
 =statistics::create_bin_based_clustering(emb, mydata.clustergrid);

std::cout << "Number of clusters " <<
 (*clustering).get_number_of_clusters() << std::endl;
*/
// STORING CLUSTER REPRESENTATIVE ON FILE

return 0;
}
