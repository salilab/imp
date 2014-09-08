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
#include <IMP/isd.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <IMP/base/Pointer.h>

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
core::MonteCarloMovers mvs;

//
// ISD PARTICLES
//
std::map<std::string, base::Pointer<Particle> > ISD_ps=
 add_ISD_particles(m,mydata,mvs);
// create list of particles from map
Particles ISD_ps_list;
std::map<std::string, base::Pointer<Particle> >::iterator itr;
for(itr = ISD_ps.begin(); itr != ISD_ps.end(); ++itr){
 ISD_ps_list.push_back((*itr).second);
}

//
// PROTEIN REPRESENTATION
//
atom::Hierarchies all_mol=
 create_representation(m,mydata,CP_ps,IL2_ps,mvs,
                       ISD_ps["SideXY"],ISD_ps["SideZ"],0);
//
// create list to link to rmf
//
atom::Hierarchies hhs;
for(unsigned int i=0;i<all_mol.size();++i){
 atom::Hierarchies hs=all_mol[i].get_children();
 for(unsigned int j=0;j<hs.size();++j) {hhs.push_back(hs[j]);}
}

//
// READ file with weights
//
Floats weights;
if(mydata.Cluster.weight){
 Float weight;
 std::ifstream weightfile;
 // open weightfile
 weightfile.open(mydata.Cluster.weightfile.c_str());
 if(weightfile.is_open()){
  // read file
  while (weightfile >> weight){weights.push_back(weight);}
  // close file
  weightfile.close();
 }
}

//
// READ LABEL FILE
//
std::ifstream labelfile;
std::string label;
std::vector<std::string> labels;
labelfile.open(mydata.Cluster.label.c_str());
while (labelfile >> label){labels.push_back(label);}
labelfile.close();

//
// CREATE ASSIGNMENT TO DEAL WITH IDENTICAL PARTICLES
//
Ints assign;
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
//
// IGNORE PARTICLE IDENTITY
//
if(!mydata.Cluster.identical){
 for(unsigned i=0; i<assign.size(); ++i){
  assign[i] = i;
 }
}
//
// IGNORE SIMMETRIES
//
if(!mydata.Cluster.symmetry){
 mydata.trs.clear();
 mydata.trs.push_back(algebra::get_identity_transformation_3d());
}

// Metric DRMS
IMP_NEW(membrane::DistanceRMSDMetric, drmsd, (cluster_ps, assign, mydata.trs,
                          ISD_ps["SideXY"], ISD_ps["SideXY"], ISD_ps["SideZ"]));

// array for scores
Floats scores;
// and cell dimension
Floats cells;
// counter
int counter = 0;

// cycle on all iterations
for(int iter=0;iter<mydata.Cluster.niter;++iter){

 // iteration string
 std::stringstream iter_str;
 iter_str << iter;

// open rmf for coordinates
 RMF::FileHandle rh =
  RMF::open_rmf_file(mydata.Cluster.trajfile+"_"+iter_str.str()+".rmf");
// read various info
 RMF::Category my_kc  = rh.get_category("my data");
 RMF::FloatKey my_key = rh.get_float_key(my_kc,"my score",true);
// linking hierarchies
 rmf::link_hierarchies(rh, hhs);
// number of frames
 unsigned int nframes=rh.get_number_of_frames();

// open rmf for ISD particles
 RMF::FileHandle rh_ISD =
  RMF::open_rmf_file(mydata.Cluster.isdtrajfile+"_"+iter_str.str()+".rmf");
// linking particles
 rmf::link_particles(rh_ISD, ISD_ps_list);
// number of frames
 unsigned int nframes_ISD=rh_ISD.get_number_of_frames();

// check number of frames are the same
 if(nframes!=nframes_ISD){exit(1);}

// cycle on frames
 for(unsigned int imc = 0; imc < nframes; ++imc){

  // load coordinates
  rmf::load_frame(rh,imc);

  // and ISD particles
  rmf::load_frame(rh_ISD,imc);

  // get score and add to list
  Float score = (rh.get_root_node()).get_value(my_key,imc);
  scores.push_back(score);

  // get cell and add to list
  cells.push_back(isd::Scale(ISD_ps["SideXY"]).get_scale()*mydata.sideMin);

  // get weight
  double weight = 1.0;
  if(mydata.Cluster.weight){weight = weights[counter];}

  // add configurations to Metric
  drmsd->add_configuration(weight);

  // increase counter
  counter += 1;
 }

 // close RMFs
 rh     = RMF::FileHandle();
 rh_ISD = RMF::FileHandle();
}

// SANITY CHECK
if(mydata.Cluster.weight && weights.size()!=counter){exit(1);}

// NOW do the clustering
base::Pointer<statistics::PartitionalClustering> pc=
 create_gromos_clustering(drmsd,mydata.Cluster.cutoff);

// calculate total population
Float pop_norm = 0.;
for(unsigned j=0; j<drmsd->get_number_of_items(); j++){
 pop_norm += drmsd->get_weight(j);
}

// a) File containing the cluster population, center, diameter and mean
FILE *centerfile;
centerfile = fopen("cluster_center.dat","w");
fprintf(centerfile,
 "#   Cluster Population  Structure   Diameter       Mean\n");
// cycle on number of clusters
for(unsigned i=0;i<pc->get_number_of_clusters();++i){
 // get list of cluster members
 Ints index=pc->get_cluster(i);
 // calculate population
 Float population = 0.;
 for(unsigned j=0;j<index.size();++j){
  population += drmsd->get_weight(index[j]);
 }
 // calculate pairwise distances and weights
 Floats dists;
 Floats weights;
 for(unsigned j=0;j<index.size()-1;++j){
  for(unsigned k=j+1;k<index.size();++k){
   double dist = drmsd->get_distance(index[j],index[k]);
   double wj = drmsd->get_weight(index[j]);
   double wk = drmsd->get_weight(index[k]);
   dists.push_back(dist);
   weights.push_back(wj*wk);
  }
 }
 Float diameter = 0.;
 Float mean = 0.;
 Float mean_norm = 0.;
 // calculate diameter and mean
 for(unsigned j=0; j<dists.size(); ++j){
  // get diameter
  if( dists[j] > diameter ){ diameter = dists[j]; }
  // get mean
  mean += dists[j] * weights[j];
  // and norm
  mean_norm += weights[j];
 }
 // to avoid nan
 if(dists.size()==0){mean_norm = 1.0;}

 fprintf(centerfile," %10u %10.6f %10d %10.6f %10.6f\n",
                      i, population / pop_norm,
                      pc->get_cluster_representative(i),
                      diameter, mean / mean_norm );
}
fclose(centerfile);

// b) File containing the distance between clusters centers
FILE *ccfile;
ccfile = fopen("cluster_distance.dat","w");
fprintf(ccfile,"#  ClusterA   ClusterB   Distance\n");
for(unsigned i=0;i<pc->get_number_of_clusters()-1;++i){
 int cl0=pc->get_cluster_representative(i);
 for(unsigned k=i+1;k<pc->get_number_of_clusters();++k){
  int cl1=pc->get_cluster_representative(k);
  fprintf(ccfile," %10u %10u %10.6f\n",i,k,drmsd->get_distance(cl0,cl1));
 }
}
fclose(ccfile);

// c) File containing the cluster index of each configuration,
//    weight and score
std::vector<unsigned> assignments(drmsd->get_number_of_items());
for(unsigned i=0;i<pc->get_number_of_clusters();++i){
 Ints members=pc->get_cluster(i);
 for(unsigned j=0;j<members.size();++j){
  assignments[members[j]]=i;
 }
}
FILE *trajfile;
trajfile = fopen("cluster_traj_score_weight.dat","w");
fprintf(trajfile,"# Structure    Cluster      Score     Weight       Cell\n");
for(unsigned i=0;i<assignments.size();++i){
 fprintf(trajfile," %10u %10d %10.4lf %10.6lf %10.6lf\n",
                    i, assignments[i], scores[i],
                    drmsd->get_weight(i), cells[i]);
}
fclose(trajfile);

return 0;
}
