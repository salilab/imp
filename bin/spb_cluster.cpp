/**
 *  \file spb_cluster.cpp
 *  \brief SPB Clustering Tool
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/membrane.h>
#include <IMP/statistics.h>
#include <IMP/rmf.h>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace IMP;
using namespace IMP::membrane;

int main(int argc, char* argv[])
{

std::string inputfile="config.ini";
int i=1;
while(i<argc){
 if(strcmp(argv[i],"-in")==0){++i; inputfile=argv[i];}
 ++i;
}

// parsing input
SPBParameters mydata=get_SPBParameters(inputfile,"0");

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
atom::Hierarchies all_mol=
 create_representation(m,mydata,bCP_ps,CP_ps,IL2_ps,mvs);
atom::Hierarchies hhs;
for(unsigned int i=0;i<all_mol.size();++i){
 atom::Hierarchies hs=all_mol[i].get_children();
 for(unsigned int j=0;j<hs.size();++j) {hhs.push_back(hs[j]);}
}
//
// READ LABEL FILE
//
std::ifstream labelfile;
std::string label;
std::vector<std::string> labels;
labelfile.open(mydata.label.c_str());
while (labelfile >> label){labels.push_back(label);}
labelfile.close();

// Create assigment
Ints assign;
//
Particles cluster_ps;
for(unsigned int j=0;j<labels.size();++j){
 for(unsigned int i=0;i<hhs.size();++i){
  Particles ps=atom::get_leaves(hhs[i]);
  for(unsigned int k=0;k<ps.size();++k){
   if(atom::Domain(ps[k])->get_name()==labels[j]){
    cluster_ps.push_back(ps[k]);
    assign.push_back(j);
   }
  }
 }
}

// Metric DRMS
IMP_NEW(membrane::DistanceRMSDMetric,drmsd,(cluster_ps,assign,mydata.trs));

// cycle on all iterations
for(unsigned iter=0;iter<mydata.niter;++iter){

 std::stringstream iter_str;
 iter_str << iter;

 RMF::FileHandle
  rh=RMF::open_rmf_file(mydata.trajfile+"_"+iter_str.str()+".rmf");

// setting hierarchies
 rmf::set_hierarchies(rh, hhs);
 unsigned int nframes=rmf::get_number_of_frames(rh,hhs[0]);

// add configurations to Metric
 for(unsigned int imc=0;imc<nframes;++imc){
  for(unsigned int i=0;i<hhs.size();++i){
   rmf::load_frame(rh,imc,hhs[i]);
  }
  drmsd->add_configuration();
 }

 // close RMF
 rh=RMF::FileHandle();

 if(mydata.cluster_time || iter==mydata.niter-1){
// do the clustering here!
  Pointer<statistics::PartitionalClustering> pc=
   statistics::create_diameter_clustering(drmsd,mydata.cluster_cut);

  std::cout << " # CHUNK            :: "
    << iter << std::endl;
  std::cout << " # CONFIGURATIONS   :: "
    << drmsd->get_number_of_items() << std::endl;
  std::cout << " # CLUSTERS         :: "
    << pc->get_number_of_clusters() << std::endl;
  std::cout << std::endl;

// only at the end, print center and traj files
  if(iter==mydata.niter-1){
// file containing the cluster population and representative config
   FILE *centerfile;
   centerfile = fopen("cluster_center.dat","w");
   fprintf(centerfile,"#       Cluster     Population      Structure\n");
   for(unsigned i=0;i<pc->get_number_of_clusters();++i){
    fprintf(centerfile," %14d %14d %14d\n",
            i, pc->get_cluster(i).size(), pc->get_cluster_representative(i));
   }
   fclose(centerfile);
// file containing the cluster index for each configuration
   std::vector<Ints> list_indexes;
   for(unsigned i=0;i<pc->get_number_of_clusters();++i){
    list_indexes.push_back(pc->get_cluster(i));
   }
   FILE *trajfile;
   trajfile = fopen("cluster_traj.dat","w");
   fprintf(trajfile,"#     Structure        Cluster\n");
   for(unsigned i=0;i<drmsd->get_number_of_items();++i){
    int index;
    for(unsigned j=0;j<list_indexes.size();++j){
     for(unsigned k=0;k<list_indexes[j].size();++k){
      if(list_indexes[j][k]==i){index=j;}
     }
    }
    fprintf(trajfile," %14d %14d\n",i,index);
   }
   fclose(trajfile);
  }
 }
}

return 0;
}
