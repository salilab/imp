/**
 *  \file DataPointsAssignment.h
 *  \brief Tools for data points assignment, after anchor point segmentation
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPMULTIFIT_DATA_POINTS_ASSIGNMENT_H
#define IMPMULTIFIT_DATA_POINTS_ASSIGNMENT_H

#include <IMP/atom/pdb.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Chain.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/rigid_bodies.h>
#include "ClusteringEngine.h"
#include "multifit_config.h"


IMPMULTIFIT_BEGIN_NAMESPACE
typedef std::pair<int, int> CPair;
typedef std::vector<CPair> CPairs;
typedef std::map<CPair, int> CEdges;
typedef std::vector<CPair> Edges;

//! Sets the assignment of particles data points into clusters according
//! to the clustering engine.
class IMPMULTIFITEXPORT DataPointsAssignment {
public:
  DataPointsAssignment(const XYZDataPoints *data,
                       const ClusteringEngine *cluster_engine);

  ~DataPointsAssignment() {}
  int get_number_of_clusters() const {return cluster_sets_.size();}
  algebra::Vector3Ds  get_cluster_vectors(int cluster_id) const;
  //Float get_cluster_xyz_diameter(int cluster_ind) const;
  const Edges *get_edges() const {return &edges_;}
  const ClusteringEngine *get_cluster_engine()
    const {return cluster_engine_;};
  IMP::algebra::Vector3Ds get_cluster_xyz(int cluster_ind) const;
protected:
  algebra::Vector3Ds set_cluster(int cluster_ind);
  void set_clusters();
  void connect_clusters(int c1, int c2);
  void set_edges();
  /*  bool are_particles_close(core::RigidBody rb1,
      core::RigidBody rb2);*/
  const XYZDataPoints *data_;
  std::vector<algebra::Vector3Ds> cluster_sets_;
  const ClusteringEngine *cluster_engine_;
  Edges edges_;
  CEdges edges_map_;
};

IMPMULTIFITEXPORT void write_chimera(
           const std::string &chimera_filename,
           const DataPointsAssignment &dpa);

IMPMULTIFITEXPORT void write_cmm(const std::string &cmm_filename,
               const std::string &marker_set_name,
               const DataPointsAssignment &dpa);
IMPMULTIFITEXPORT std::pair<algebra::Vector3Ds,
                            CEdges> read_cmm(const std::string &cmm_filename);

IMPMULTIFITEXPORT void write_max_cmm(const std::string &cmm_filename,
                   em::DensityMap *dmap,
                   const std::string &marker_set_name,
                   const DataPointsAssignment &dpa);
IMPMULTIFITEXPORT void write_pdb(const std::string &pdb_filename,
               const DataPointsAssignment &dpa);

void write_segments_as_pdb(const DataPointsAssignment &dpa,
                           const std::string &filename);

void write_segment_as_pdb(const DataPointsAssignment &dpa,
                          int segment_id,
                          const std::string &filename);
IMPMULTIFITEXPORT
void write_segments_as_mrc(em::DensityMap *dmap,
const DataPointsAssignment &dpa,
Float resolution, Float apix,
const std::string &filename);

IMPMULTIFITEXPORT
void write_segment_as_mrc(em::DensityMap *dmap,
const DataPointsAssignment &dpa,int segment_id,
Float resolution, Float apix,const std::string &filename);

algebra::Vector3D get_segment_maximum(const DataPointsAssignment &dpa,
                                      em::DensityMap *dmap, int segment_id);
algebra::Vector3D get_segment_maximum(const DataPointsAssignment &dpa,
                                      DensGrid *dmap, int segment_id);
IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_DATA_POINTS_ASSIGNMENT_H */
