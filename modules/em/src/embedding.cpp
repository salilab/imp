/**
 *  \file point_clustering.cpp
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/em/embedding.h>

IMPEM_BEGIN_NAMESPACE

HighDensityEmbedding::HighDensityEmbedding(em::DensityMap *dm,
                                           double threshold):
  statistics::Embedding("HighDensityEmbedding of "+dm->get_name()) {
  for (int i=0; i< dm->get_number_of_voxels(); ++i) {
    if (dm->get_value(i) > threshold) {
      algebra::VectorD<3> v(dm->get_location_in_dim_by_voxel(i, 0),
                          dm->get_location_in_dim_by_voxel(i, 1),
                          dm->get_location_in_dim_by_voxel(i, 2));
      points_.push_back(v);
    }
  }
}

Floats HighDensityEmbedding::get_point(unsigned int i) const {
  return Floats(points_[i].coordinates_begin(),
                points_[i].coordinates_end());
}

unsigned int HighDensityEmbedding::get_number_of_points() const {
  return points_.size();
}

void HighDensityEmbedding::do_show(std::ostream &out) const {
  out << points_.size()
      << " points.";
  out << std::endl;
}



IMPEM_END_NAMESPACE
