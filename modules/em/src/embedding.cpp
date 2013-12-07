/**
 *  \file point_clustering.cpp
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/em/embedding.h>

IMPEM_BEGIN_NAMESPACE

HighDensityEmbedding::HighDensityEmbedding(em::DensityMap *dm, double threshold)
    : statistics::Embedding("HighDensityEmbedding of " + dm->get_name()) {
  for (int i = 0; i < dm->get_number_of_voxels(); ++i) {
    if (dm->get_value(i) > threshold) {
      algebra::Vector3D v(dm->get_location_in_dim_by_voxel(i, 0),
                          dm->get_location_in_dim_by_voxel(i, 1),
                          dm->get_location_in_dim_by_voxel(i, 2));
      points_.push_back(v);
    }
  }
}

algebra::VectorKD HighDensityEmbedding::get_point(unsigned int i) const {
  return algebra::VectorKD(points_[i].coordinates_begin(),
                           points_[i].coordinates_end());
}

unsigned int HighDensityEmbedding::get_number_of_items() const {
  return points_.size();
}

IMPEM_END_NAMESPACE
