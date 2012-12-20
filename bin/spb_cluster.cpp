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
IMP_NEW(container::ListSingletonContainer,CP_ps,(m));
IMP_NEW(container::ListSingletonContainer,IL2_ps,(m));
// List of Movers for MC, not used here
core::Movers mvs;
//
// PROTEIN REPRESENTATION
//
atom::Hierarchies all_mol=
 create_representation(m,mydata,CP_ps,IL2_ps,mvs);
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
for(int iter=0;iter<mydata.niter;++iter){

 std::stringstream iter_str;
 iter_str << iter;

 RMF::FileHandle
  rh=RMF::open_rmf_file(mydata.trajfile+"_"+iter_str.str()+".rmf");
 RMF::Category my_kc  = rh.get_category("my data");
 RMF::FloatKey my_key = rh.get_float_key(my_kc,"my score",true);

// linking hierarchies
 rmf::link_hierarchies(rh, hhs);
 unsigned int nframes=rh.get_number_of_frames();

// add configurations to Metric
 for(unsigned int imc=0;imc<nframes;++imc){
  rmf::load_frame(rh,imc);
  double weight=1.0;
  if(mydata.cluster_weight){
   weight=exp(-((rh.get_root_node()).get_value(my_key,imc)/mydata.MC.tmin));
  }
  drmsd->add_configuration(weight);
 }

 // close RMF
 rh=RMF::FileHandle();
}

// NOW do the clustering
Pointer<statistics::PartitionalClustering> pc=
 create_gromos_clustering(drmsd,mydata.cluster_cut);

// a) File containing the cluster population, center and diameter
FILE *centerfile;
centerfile = fopen("cluster_center.dat","w");
fprintf(centerfile,
 "#       Cluster     Population      Structure       Diameter\n");
for(unsigned i=0;i<pc->get_number_of_clusters();++i){
 double diameter=0.0;
 Ints index=pc->get_cluster(i);
 for(unsigned j=0;j<index.size()-1;++j){
  for(unsigned k=j+1;k<index.size();++k){
   double dist=drmsd->get_distance(index[j],index[k]);
   if(dist>diameter){diameter=dist;}
  }
 }
 fprintf(centerfile," %14u %14u %14d %14.6f\n",
                      i, (unsigned)index.size(),
                      pc->get_cluster_representative(i), diameter);
}
fclose(centerfile);

// b) File containing the distance between clusters centers
FILE *ccfile;
ccfile = fopen("cluster_distance.dat","w");
fprintf(ccfile,"#      ClusterA       ClusterB       Distance\n");
for(unsigned i=0;i<pc->get_number_of_clusters()-1;++i){
 int cl0=pc->get_cluster_representative(i);
 for(unsigned k=i+1;k<pc->get_number_of_clusters();++k){
  int cl1=pc->get_cluster_representative(k);
  fprintf(ccfile," %14u %14u %14.6f\n",i,k,drmsd->get_distance(cl0,cl1));
 }
}
fclose(ccfile);

// c) File containing the cluster index of each configuration
std::vector<unsigned> assignments(drmsd->get_number_of_items());
for(unsigned i=0;i<pc->get_number_of_clusters();++i){
 Ints members=pc->get_cluster(i);
 for(unsigned j=0;j<members.size();++j){
  assignments[members[j]]=i;
 }
}
FILE *trajfile;
trajfile = fopen("cluster_traj.dat","w");
fprintf(trajfile,"#     Structure        Cluster\n");
for(unsigned i=0;i<assignments.size();++i){
 fprintf(trajfile," %14u %14d\n",i,assignments[i]);
}
fclose(trajfile);

return 0;
}
