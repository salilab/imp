/**
 *  \file MapScorer   \brief A class for fast cc scoring of docking models
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include "MapScorer.h"

#include <IMP/em/converters.h>

MapScorer::MapScorer(IMP::Particles& rec_particles,
                     IMP::Particles& lig_particles,
                     const IMP::em::DensityMap& map) :
  complex_map_(map) {

  float resolution = complex_map_.get_header()->get_resolution();
  float voxel_size = complex_map_.get_header()->get_spacing();

  // generate receptor and ligand maps
  receptor_map_ =
    IMP::em::particles2density(rec_particles, resolution, voxel_size);
  ligand_map_ =
    IMP::em::particles2density(lig_particles, resolution, voxel_size);
  docked_complex_map_ =
    new IMP::em::SampledDensityMap(*complex_map_.get_header());
  docked_complex_map_->copy_map(&complex_map_);
}

MapScorer::MapScorer(IMP::Particles& particles, const IMP::em::DensityMap& map):
  complex_map_(map) {
  float resolution = complex_map_.get_header()->get_resolution();
  float voxel_size = complex_map_.get_header()->get_spacing();

  // generate complex map
  receptor_map_ = IMP::em::particles2density(particles, resolution, voxel_size);
  docked_complex_map_ =
    new IMP::em::SampledDensityMap(*complex_map_.get_header());
  docked_complex_map_->copy_map(&complex_map_);
}

float MapScorer::score(const IMP::algebra::Transformation3D& rec_trans,
                       const IMP::algebra::Transformation3D& lig_trans) const {
  float voxel_data_threshold = complex_map_.get_header()->dmin;
  IMP::em::DensityMap* transformed_complex_map =
    get_transformed(rec_trans, lig_trans);
  float cc =
    IMP::em::CoarseCC::cross_correlation_coefficient(&complex_map_,
                                                     transformed_complex_map,
                                                     voxel_data_threshold);
  return cc;
}

float MapScorer::score(const IMP::algebra::Transformation3D& trans) const {
  float voxel_data_threshold = complex_map_.get_header()->dmin;
  IMP::em::DensityMap* transformed_complex_map = get_transformed(trans);
  float cc =
    IMP::em::CoarseCC::cross_correlation_coefficient(&complex_map_,
                                                     transformed_complex_map,
                                                     voxel_data_threshold);
  return cc;
}


IMP::em::DensityMap* MapScorer::get_transformed(
                        const IMP::algebra::Transformation3D& trans) const {
  IMP::algebra::Transformation3D trans_inverse = trans.get_inverse();
  for(unsigned int i=0; i<docked_complex_map_->get_number_of_voxels(); i++) {
    IMP::algebra::Vector3D voxel_coordinate =
      docked_complex_map_->get_location_by_voxel(i);
    IMP::algebra::Vector3D point = trans_inverse * voxel_coordinate;
    double d = IMP::em::get_density(receptor_map_, point);
    docked_complex_map_->set_value(i, d);
  }
  docked_complex_map_->calcRMS();
  return docked_complex_map_;
}


IMP::em::DensityMap* MapScorer::get_transformed(
                        const IMP::algebra::Transformation3D& rec_trans,
                        const IMP::algebra::Transformation3D& lig_trans) const {
  // inverse transformations, since we are sampling the complex map
  // by going back to receptor and ligand maps
  IMP::algebra::Transformation3D rec_trans_inverse = rec_trans.get_inverse();
  IMP::algebra::Transformation3D lig_trans_inverse = lig_trans.get_inverse();

  for(unsigned int i=0; i<docked_complex_map_->get_number_of_voxels(); i++) {
    IMP::algebra::Vector3D voxel_coordinate =
      docked_complex_map_->get_location_by_voxel(i);
    IMP::algebra::Vector3D point1 = rec_trans_inverse * voxel_coordinate;
    IMP::algebra::Vector3D point2 = lig_trans_inverse * voxel_coordinate;
    double d1 = IMP::em::get_density(receptor_map_, point1);
    double d2 = IMP::em::get_density(ligand_map_, point2);
    double d = d1+d2;//std::max(d1, d2);
    docked_complex_map_->set_value(i, d);
  }
  docked_complex_map_->calcRMS();
  return docked_complex_map_;
}
