/**
 *  \file DataPointsAssignment.cpp
 *  \brief Tools for data points assignment, after anchor point segmentation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/DataPointsAssignment.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/em/converters.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/container/ClosePairContainer.h>
#include <IMP/core/RigidClosePairsFinder.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/container/PairsRestraint.h>
#include <IMP/core/ChildrenRefiner.h>
#include <IMP/display/particle_geometry.h>
#include <IMP/display/ChimeraWriter.h>
#include <IMP/em/Voxel.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/core/LeavesRefiner.h>
#include <IMP/multifit/density_analysis.h>
#include <IMP/statistics/internal/ClusteringEngine.h>
#include <IMP/statistics/internal/DataPoints.h>
#include <algorithm>

IMPMULTIFIT_BEGIN_NAMESPACE
namespace {
bool sort_data_points_first_larger_than_second(
            const std::pair<float,algebra::Vector3D> &a,
            const std::pair<float,algebra::Vector3D> &b) {
  return a.first>b.first;
}
}
DataPointsAssignment::DataPointsAssignment
(const IMP::statistics::internal::XYZDataPoints *data,
 const IMP::statistics::internal::ClusteringEngine *cluster_engine) {
    cluster_engine_ = cluster_engine;
    data_ = data;
    IMP_USAGE_CHECK(data_->get_number_of_data_points() > 0,
                    "DataPointsAssignment::DataPointsAssignment zero points,"<<
                    "nothing to assign");
    IMP_LOG_VERBOSE("going to set clusters"<< std::endl);
    set_clusters();
    IMP_LOG_VERBOSE("going to set edges"<< std::endl);
    set_edges();
    IMP_LOG_VERBOSE("finish assignment"<< std::endl);
  }

algebra::Vector3Ds
DataPointsAssignment::get_cluster_vectors(int cluster_id) const {
  IMP_USAGE_CHECK(
    static_cast<unsigned int>(cluster_id)<cluster_sets_.size(),
    "DataPointsAssignment::get_cluster cluster index is out of range");
  return cluster_sets_[cluster_id];
}

algebra::Vector3Ds
  DataPointsAssignment::set_cluster(int cluster_ind) {
  //remove outliers
  Pointer<Model> mdl = new Model();
  ParticlesTemp full_set;//all points of the cluster
  for (int i=0;i<data_->get_number_of_data_points();i++) {
    if (cluster_engine_->is_part_of_cluster(i,cluster_ind)) {
      core::XYZR x = core::XYZR::setup_particle(new Particle(mdl),
                                 algebra::Sphere3D(data_->get_vector(i),1));
      atom::Mass::setup_particle(x,1);
      full_set.push_back(x);
    }
  }
  Pointer<em::DensityMap> full_map = em::particles2density(full_set,3,1.5);
  //map the particles to their voxels
  std::map<long,algebra::Vector3D> voxel_particle_map;
  for(unsigned int i=0;i<full_set.size();i++) {
    algebra::Vector3D v=core::XYZ(full_set[i]).get_coordinates();
    voxel_particle_map[
                       full_map->get_voxel_by_location(v)]=v;
  }
  full_map->set_was_used(true);
  IntsList conn_comp=get_connected_components(full_map,0.001,0.8);
  IMP_LOG_TERSE("Number of connected components:"<<conn_comp.size()<<std::endl);
  unsigned int num_elements=0;
  for(unsigned int i=0;i<conn_comp.size();i++) {
    num_elements+=conn_comp[i].size();
  }
  //use only connected components that consist of at least 40% of the density
  algebra::Vector3Ds cluster_set;
  for(unsigned int i=0;i<conn_comp.size();i++) {
    IMP_LOG_TERSE(
            "====connected component:"<<i<<" is of size "
            <<conn_comp[i].size()<< " " << full_set.size()
            <<" "<< num_elements<<" "<<0.4*num_elements
            <<" "<< (conn_comp[i].size()<(0.4*num_elements))<<std::endl);
    if (conn_comp[i].size()<(0.4*num_elements))
      continue;
    IMP_LOG_TERSE(
            "====connected component:"<<i<<" is being considered"<<std::endl);
    for (unsigned int j=0;j<conn_comp[i].size();j++) {
      if (voxel_particle_map.find(conn_comp[i][j]) != voxel_particle_map.end())
        {
          cluster_set.push_back(voxel_particle_map[conn_comp[i][j]]);
        }}
  }
  //here we assume properties are only xyz
  IMP::statistics::internal::Array1DD cen
    =cluster_engine_->get_center(cluster_ind);
  cluster_set.push_back(algebra::Vector3D(cen[0],cen[1],cen[2]));
  IMP_LOG_VERBOSE("setting cluster " <<
          cluster_ind << " with " << cluster_set.size()
          << " points " << std::endl);
  return cluster_set;
}

/** This function creates list of particles for each cluster
 */
void DataPointsAssignment::set_clusters() {
  cluster_sets_.clear();
  for(int i=0;i<cluster_engine_->get_number_of_clusters();i++) {
    cluster_sets_.push_back(set_cluster(i));
  }
}

void DataPointsAssignment::set_edges(double voxel_size) {
  //create projected density maps for each cluster
  std::vector<Pointer<em::SampledDensityMap> > dmaps;
  std::vector<algebra::BoundingBox3D> boxes;
  Pointer<Model> mdl = new Model();
  for(int i=0;i<cluster_engine_->get_number_of_clusters();i++) {
    algebra::Vector3Ds vecs =get_cluster_vectors(i);
    ParticlesTemp ps(vecs.size());
    for(unsigned int j=0;j<vecs.size();j++) {
      core::XYZR x = core::XYZR::setup_particle(new Particle(mdl),
                          algebra::Sphere3D(vecs[j],voxel_size));
      atom::Mass::setup_particle(x,3.);
      ps[j]=x;
    }
    boxes.push_back(core::get_bounding_box(core::XYZRs(ps)));
    Pointer<em::SampledDensityMap> segment_map =
      em::particles2density(ps,voxel_size*1.5,voxel_size);
    segment_map->set_was_used(true);
    dmaps.push_back(segment_map);
  }//end create maps

  //define edges
  for(int i=0;i<cluster_engine_->get_number_of_clusters();i++) {
    for(int j=i+1;j<cluster_engine_->get_number_of_clusters();j++) {
      if (!algebra::get_interiors_intersect(boxes[i],boxes[j]))
        continue;
      algebra::Vector3Ds vecs = get_cluster_vectors(j);
      //check if the vectors are inside the i'th map
      bool touching=false;
      for(unsigned int k=0;(k<vecs.size())&&(!touching);k++) {
        if (!dmaps[i]->is_part_of_volume(vecs[k]))
          continue;
        if (dmaps[i]->get_value(vecs[k])>0.01)
          touching=true;
      }
      if (touching) {
        edges_.push_back(IntPair(i,j));
      }
    }//for j
  }//for i
}


void DataPointsAssignment::connect_clusters(int c1, int c2) {
  IMP_USAGE_CHECK(c1 != c2,
                  "DataPointsAssignment::connect_centers can"<<
                  " not connect a cluster to itself");
  int min_c = std::min(c1,c2);
  int max_c = std::max(c1,c2);
  if (edges_map_.find(IntPair(min_c,max_c)) == edges_map_.end()) {
    edges_map_[IntPair(min_c,max_c)]=1;
    edges_.push_back(IntPair(min_c,max_c));
  }
}

void write_segments_as_pdb(const DataPointsAssignment &dpa,
                           const std::string &filename){
  for( int i=0;i<dpa.get_number_of_clusters();i++) {
    std::stringstream filename_full;
    filename_full<<filename<<"_"<<i<<".pdb";
    write_segment_as_pdb(dpa,i,filename_full.str());
  }
}

void write_segment_as_pdb(const DataPointsAssignment &dpa,
                          int segment_id,
                          const std::string &filename) {
  std::ofstream f;
  f.open(filename.c_str());
  algebra::Vector3Ds cluster_ps=dpa.get_cluster_vectors(segment_id);
  for(unsigned int i=0;i<cluster_ps.size();i++) {
    f<<atom::get_pdb_string(cluster_ps[i],
                             i,atom::AT_CA,atom::ALA,'A',i);
  }
}


void write_segment_as_mrc(em::DensityMap *dmap,
const DataPointsAssignment &dpa,int segment_id,
Float , Float ,const std::string &filename) {
  Pointer<em::DensityMap> segment_map(
new em::DensityMap(*(dmap->get_header())));
  segment_map->reset_data(0.);
  //  segment_map->update_voxel_size(apix);
  algebra::Vector3Ds vecs =dpa.get_cluster_vectors(segment_id);
  for(unsigned int i=0;i<vecs.size();i++) {
    segment_map->set_value(
vecs[i][0],vecs[i][1],vecs[i][2],dmap->get_value(vecs[i]));
  }
  em::write_map(segment_map,filename.c_str(),new em::MRCReaderWriter());
  segment_map=static_cast<em::DensityMap*>(nullptr);
}

algebra::Vector3D get_segment_maximum(const DataPointsAssignment &dpa,
                                      em::DensityMap *dmap,
                                      int segment_id){
  algebra::Vector3Ds vecs =dpa.get_cluster_xyz(segment_id);
  std::vector<std::pair<float,algebra::Vector3D> > data_for_sorting;
  for(algebra::Vector3Ds::iterator it = vecs.begin();
      it != vecs.end(); it++) {
    data_for_sorting.push_back(
       std::pair<float,algebra::Vector3D>(dmap->get_value(*it),*it));
  }
  std::sort(data_for_sorting.begin(),data_for_sorting.end(),
            sort_data_points_first_larger_than_second);
  return data_for_sorting[0].second;
}

algebra::Vector3D get_segment_maximum(const DataPointsAssignment &dpa,
                                      DensGrid *dmap,
                                      int segment_id){
  algebra::Vector3Ds vecs =dpa.get_cluster_xyz(segment_id);
  std::vector<std::pair<float,algebra::Vector3D> > data_for_sorting;
  for(algebra::Vector3Ds::iterator it = vecs.begin();
      it != vecs.end(); it++) {
    data_for_sorting.push_back(
   std::pair<float,algebra::Vector3D>((*dmap)[dmap->get_nearest_index(*it)],
                                     *it));
  }
  std::sort(data_for_sorting.begin(),data_for_sorting.end(),
            sort_data_points_first_larger_than_second);
  return data_for_sorting[0].second;
}

/*
void write_max_cmm(const std::string &cmm_filename,
                   em::DensityMap *dmap,
                   const std::string &marker_set_name,
                   const DataPointsAssignment &dpa) {
  algebra::Vector3Ds centers;
  Floats radii;
  for( int i=0;i<dpa.get_number_of_clusters();i++) {
    algebra::Vector3D xyz = get_segment_maximum(dpa,dmap,i);
    centers.push_back(xyz);
    radii.push_back(3);
  }
  std::ofstream out;
  out.open(cmm_filename.c_str(),std::ios::out);
  write_cmm_helper(out,marker_set_name,centers,*(dpa.get_edges()),radii);
  out.close();
  }*/

void write_pdb(const std::string &pdb_filename,
               const DataPointsAssignment &dpa) {
  IMP_LOG_VERBOSE("going to write pdb " <<
          pdb_filename <<" with : " <<
          dpa.get_number_of_clusters() <<" clusters " << std::endl);
  std::ofstream out;
  out.open(pdb_filename.c_str(),std::ios::out);
  algebra::Vector3Ds centers;
  for( int i=0;i<dpa.get_number_of_clusters();i++) {
    IMP::statistics::internal::Array1DD xyz =
      dpa.get_cluster_engine()->get_center(i);
    centers.push_back(algebra::Vector3D(xyz[0],xyz[1],xyz[2]));
    out<<atom::get_pdb_string(centers[i],i,
                          atom::AT_CA,atom::ALA,'A',i);
  }
  out.close();
}

void write_segments_as_mrc(em::DensityMap *dmap,
                           const DataPointsAssignment &dpa,
                           Float resolution, Float apix,
                           Float threshold,
                           const std::string &filename){
  //all filenames
  std::vector<std::string> mrc_filenames;
  for( int i=0;i<dpa.get_number_of_clusters();i++) {
    std::stringstream filename_full;
    filename_full<<filename<<"_"<<i<<".mrc";
    mrc_filenames.push_back(filename_full.str());
    write_segment_as_mrc(dmap,dpa,i,resolution,apix,filename_full.str());
  }
  //generate a chimera command file to load all of the mrc files
  std::ofstream cmd_s;
  std::stringstream cmd_s_fn;
  cmd_s_fn<<"load_configuration.cmd";
  cmd_s.open(cmd_s_fn.str().c_str());
  for (int i=0;i<(int)mrc_filenames.size();i++) {
    cmd_s<<"open "<< mrc_filenames[i]<<std::endl;
  }
  cmd_s<<"vol all level "<<threshold<<std::endl;
  cmd_s.close();
}

algebra::Vector3Ds DataPointsAssignment::get_cluster_xyz(int cluster_ind)
  const {
  IMP_USAGE_CHECK(static_cast<unsigned int>(cluster_ind)<cluster_sets_.size(),
      "DataPointsAssignment::get_cluster_xyz cluster index is out of range\n");
  algebra::Vector3Ds xyz;
  for(algebra::Vector3Ds::const_iterator
        it = cluster_sets_[cluster_ind].begin();
      it != cluster_sets_[cluster_ind].end();it++) {
    xyz.push_back(*it);
  }
  return xyz;
}

// Float DataPointsAssignment::get_cluster_xyz_diameter(int cluster_ind) const {
//     return get_diameter(get_cluster_xyz(cluster_ind));
// }


// void write_chimera(
//      const std::string &chimera_filename,
//      const DataPointsAssignment &dpa) {
//   std::vector<display::SphereGeometry *> gs =
//     display_helper(dpa);
//   IMP_NEW(display::ChimeraWriter,w,(chimera_filename));
//   for( int i=0;i<gs.size();i++){
//     w->add_geometry(gs[i]);
//   }
//   w=nullptr;
// }

IMPMULTIFIT_END_NAMESPACE
