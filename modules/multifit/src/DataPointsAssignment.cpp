/**
 *  \file DataPointsAssignment.cpp
 *  \brief Tools for data points assignment, after anchor point segmentation
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/multifit/DataPointsAssignment.h>
#include <IMP/em/MRCReaderWriter.h>
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
#include <algorithm>

IMPMULTIFIT_BEGIN_NAMESPACE


// std::vector<display::SphereGeometry *> display_helper(
//      const DataPointsAssignment &dpa) {
//   std::vector<display::SphereGeometry *> gs;
//   for( int i=0;i<dpa.get_number_of_clusters();i++) {
//     Array1DD xyz = dpa.get_cluster_engine()->get_center(i);
//     algebra::Sphere3D sph=
//       algebra::get_enclosing_sphere(dpa.get_cluster_vectors(i));
//     sph.set_center(algebra::Vector3D(xyz[0],xyz[1],xyz[2]));
//     IMP_NEW(display::SphereGeometry,g,(sph));
//     //    g->set_color(display::get_display_color(10*i));
//     std::stringstream ss;
//     ss<<"cluster_"<<i;
//     g->set_name(ss.str());
//     gs.push_back(g);
//   }
//   return gs;
// }


void write_cmm_helper(std::ostream &out,
                      const std::string &marker_set_name,
                      const IMP::algebra::Vector3Ds &nodes,
                      const Edges &edges, IMP::Floats radii){
  Float x,y,z,radius;
  out << "<marker_set name=\"" <<marker_set_name << "\">"<<std::endl;
  for(unsigned int i=0;i<nodes.size();i++) {
    x = nodes[i][0];
    y = nodes[i][1];
    z = nodes[i][2];
    std::string name="";
    if(radii.size() >0 ) {
      radius = radii[i];
    }
    out << "<marker id=\"" << i << "\""
        << " x=\"" << x << "\""
        << " y=\"" << y << "\""
        << " z=\"" << z << "\""
        << " radius=\"" << radius << "\"/>" << std::endl;
    }
  for(Edges::const_iterator it = edges.begin(); it != edges.end();it++) {
    out << "<link id1= \"" << it->first
        << "\" id2=\""     << it->second
        << "\" radius=\"1.0\"/>" << std::endl;
  }
  out << "</marker_set>" << std::endl;
}

bool sort_data_points_first_larger_than_second(
            const std::pair<float,algebra::Vector3D> &a,
            const std::pair<float,algebra::Vector3D> &b) {
  return a.first>b.first;
}
DataPointsAssignment::DataPointsAssignment(
                      const XYZDataPoints *data,
                      const ClusteringEngine *cluster_engine) {
    cluster_engine_ = cluster_engine;
    data_ = data;
    IMP_USAGE_CHECK(data_->get_number_of_data_points() > 0,
                    "DataPointsAssignment::DataPointsAssignment zero points,"<<
                    "nothing to assign");
    IMP_LOG(VERBOSE,"going to set clusters"<< std::endl);
    set_clusters();
    IMP_LOG(VERBOSE,"going to set edges"<< std::endl);
    set_edges();
    IMP_LOG(VERBOSE,"finish assignment"<< std::endl);
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
  algebra::Vector3Ds cluster_set;
  for (int i=0;i<data_->get_number_of_data_points();i++) {
    if (cluster_engine_->is_part_of_cluster(i,cluster_ind)) {
      cluster_set.push_back(data_->get_vector(i));
    }
  }
  //here we assume properties are only xyz
  Array1DD cen=cluster_engine_->get_center(cluster_ind);
  cluster_set.push_back(algebra::Vector3D(cen[0],cen[1],cen[2]));
  IMP_LOG(VERBOSE,"setting cluster " <<
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

void DataPointsAssignment::set_edges() {
  //create bounding boxes for particles
  algebra::BoundingBox3Ds bb;
  for(int i=0;i<cluster_engine_->get_number_of_clusters();i++) {
    bb.push_back(algebra::BoundingBox3D(get_cluster_vectors(i)));
  }
  //define edges
  for(int i=0;i<cluster_engine_->get_number_of_clusters();i++) {
    for(int j=i+1;j<cluster_engine_->get_number_of_clusters();j++) {
      if (algebra::get_interiors_intersect(bb[i],bb[j])){
        edges_.push_back(CPair(i,j));
      }
    }
  }
}


void DataPointsAssignment::connect_clusters(int c1, int c2) {
  IMP_USAGE_CHECK(c1 != c2,
                  "DataPointsAssignment::connect_centers can"<<
                  " not connect a cluster to itself");
  int min_c = std::min(c1,c2);
  int max_c = std::max(c1,c2);
  if (edges_map_.find(CPair(min_c,max_c)) == edges_map_.end()) {
    edges_map_[CPair(min_c,max_c)]=1;
    edges_.push_back(CPair(min_c,max_c));
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
Float resolution, Float apix,const std::string &filename) {
  Pointer<em::DensityMap> segment_map(
new em::DensityMap(*(dmap->get_header())));
  segment_map->reset_data(0.);
  algebra::Vector3Ds vecs =dpa.get_cluster_vectors(segment_id);
  for(unsigned int i=0;i<vecs.size();i++) {
    segment_map->set_value(
vecs[i][0],vecs[i][1],vecs[i][2],dmap->get_value(vecs[i]));
  }
  em::MRCReaderWriter mrw;
  em::write_map(segment_map,filename.c_str(),mrw);
  segment_map=NULL;
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

void write_cmm(const std::string &cmm_filename,
               const std::string &marker_set_name,
               const DataPointsAssignment &dpa) {
  algebra::Vector3Ds centers;
  Floats radii;
  float sphere_scale=0.6;
  for( int i=0;i<dpa.get_number_of_clusters();i++) {
    Array1DD xyz = dpa.get_cluster_engine()->get_center(i);
    centers.push_back(algebra::Vector3D(xyz[0],xyz[1],xyz[2]));
    radii.push_back(
     algebra::get_enclosing_sphere(dpa.get_cluster_vectors(i)).get_radius()
     *sphere_scale);
  }
  std::ofstream out;
  out.open(cmm_filename.c_str(),std::ios::out);
  write_cmm_helper(out,marker_set_name,centers,*(dpa.get_edges()),radii);
  out.close();
}

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
}

void write_pdb(const std::string &pdb_filename,
               const DataPointsAssignment &dpa) {
  IMP_LOG(VERBOSE,"going to write pdb " <<
          pdb_filename <<" with : " <<
          dpa.get_number_of_clusters() <<" clusters " << std::endl);
  std::ofstream out;
  out.open(pdb_filename.c_str(),std::ios::out);
  algebra::Vector3Ds centers;
  for( int i=0;i<dpa.get_number_of_clusters();i++) {
    Array1DD xyz = dpa.get_cluster_engine()->get_center(i);
    centers.push_back(algebra::Vector3D(xyz[0],xyz[1],xyz[2]));
    out<<atom::get_pdb_string(centers[i],i,
                          atom::AT_CA,atom::ALA,'A',i);
  }
  out.close();
}

void write_segments_as_mrc(em::DensityMap *dmap,
const DataPointsAssignment &dpa,
Float resolution, Float apix,
const std::string &filename){
  for( int i=0;i<dpa.get_number_of_clusters();i++) {
    std::stringstream filename_full;
    filename_full<<filename<<"_"<<i<<".mrc";
    write_segment_as_mrc(dmap,dpa,i,resolution,apix,filename_full.str());
  }
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
//   w=NULL;
// }

IMPMULTIFIT_END_NAMESPACE
