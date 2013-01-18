/**
 * \file MapDistanceTransform.h \brief class for computing a distance
 * transform of the density map
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_MAP_DISTANCE_TRANSFORM_H
#define IMPEM_MAP_DISTANCE_TRANSFORM_H

#include <IMP/em/em_config.h>
#include <IMP/em/DensityMap.h>

IMPEM_BEGIN_NAMESPACE

//! Class for getting the distance from the map envelope
/**
   The class creates and stores a conversion of density map into
   a distance transform. The envelope voxels (that are determined
   using density_threshold) are zero distance. The rest of the voxels
   hold the distance to the closest envelop voxel. Positive distance
   means inside the object, negative - outside.
 */
class IMPEMEXPORT MapDistanceTransform : public IMP::em::DensityMap {
public:
  /** init the distance transform
      \param [in] map - input density map
      \param [in] density_threshold - a threshold that detemines
                                      the envelope of the map
      \param [in] max_distance - how far from the envelope to compute
                                 the transform
   */
  MapDistanceTransform(IMP::em::DensityMap* map,
                       float density_threshold,
                       float max_distance);

  /** get the distance from object envelope
      (-max_float is returned if the point is outside the grid)
  */
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

IMPEM_END_NAMESPACE

#endif /* IMPEM_MAP_DISTANCE_TRANSFORM_H */
