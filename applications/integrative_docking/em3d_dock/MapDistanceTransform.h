/**
 * \file MapDistanceTransform \brief class for computing a distance
 * transform of the density map
 *
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMP_MAP_DISTANCE_TRANSFORM_H
#define IMP_MAP_DISTANCE_TRANSFORM_H

#include <IMP/em/DensityMap.h>

//! Class for getting the distance from the map envelope
/** positive distance means inside the object, negative - outside
 */
class MapDistanceTransform : public IMP::em::DensityMap {
public:
  MapDistanceTransform() {}

  MapDistanceTransform(const DensityMap& map, float density_threshold,
                       float max_distance);

  // distance from object envelope (-max_float is returned if the point
  // is outside the grid)
  float get_distance_from_envelope(const IMP::algebra::Vector3D& v) const {
    long index = get_voxel_by_location(v);
    if(index >= 0 && index < get_number_of_voxels()) return data_[index];
    return -std::numeric_limits<float>::max();
  }

 private:
  void compute_distance_transform();

 protected:
  // cube neighbors, there are 26 neighbors for each voxel
  std::vector<long> neighbors_;
  float max_distance_; // distance up to which the grid distances are computed
};

#endif /* IMP_MAP_DISTANCE_TRANSFORM_H */
