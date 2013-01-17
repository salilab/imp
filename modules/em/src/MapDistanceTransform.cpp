/**
 * \file MapDistanceTransform \brief class for computing a distance
 * transform of the density map
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/em/MapDistanceTransform.h>

IMPEM_BEGIN_NAMESPACE

MapDistanceTransform::MapDistanceTransform(IMP::em::DensityMap* map,
                                           float density_threshold,
                                           float max_distance) {
  max_distance_ = max_distance;

  // create a new map in the same orientation as the input density map
  header_ = *map->get_header();
  set_void_map(header_.get_nx(), header_.get_ny(), header_.get_nz());
  set_origin(map->get_origin());
  update_voxel_size(header_.get_spacing());
  set_name(std::string("MapDistanceTransform ") + map->get_name());
  calc_all_voxel2loc();

  // this is not a regular density map, so normalization is meaningless
  normalized_ = false;
  rms_calculated_ = false;

  // init neighbour voxels offsets
  for(int x=-1; x<=1; x++) {
    for(int y=-1; y<=1; y++) {
      for(int z=-1; z<=1; z++) {
        if (x == 0 && y == 0 && z == 0) continue;
        neighbors_.push_back(z*header_.get_nx()*header_.get_ny() +
                             y*header_.get_nx() + x);
      }
    }
  }

  // init grid: voxels below threshold are background voxels (negative value)
  // voxels above threshold are object voxels (positive value)
  for(long l=0; l<get_number_of_voxels(); l++) {
    if(map->get_value(l) >= density_threshold) {
      data_[l] = std::numeric_limits<float>::max();
    } else {
      data_[l] = -std::numeric_limits<float>::max();
    }
  }
  compute_distance_transform();
}

void MapDistanceTransform::compute_distance_transform() {
  std::vector<IMP::algebra::Vector3D> envelope_voxels_centers;
  // store voxel indices for current and next layer
  std::vector<long> layer1,layer2, *curr_layer = &layer1, *next_layer = &layer2;
  // store closest envelope voxel for current and next layer
  std::vector<IMP::algebra::Vector3D*> envelope_voxels1, envelope_voxels2;
  std::vector<IMP::algebra::Vector3D*> *curr_envelope_voxels,
    *next_envelope_voxels;
  curr_envelope_voxels = &envelope_voxels1;
  next_envelope_voxels = &envelope_voxels2;
  // stores the layer number
  std::vector<short> grid_layer(get_number_of_voxels(),(short)-1);

  // Find envelope voxels and mark zero in the grid. Envelope voxel is a
  // voxel belonging to map object with one or more background voxel
  // neighbours
  for(long l=0; l<get_number_of_voxels(); l++) {
    if(data_[l] > 0.0) { // l belongs to object
      // iterate neighbours
      for(unsigned int j=0; j<neighbors_.size(); j++) {
        long nindex = l + neighbors_[j];
        if(nindex >= 0 && nindex < get_number_of_voxels() &&
           data_[nindex] < 0.0) { //envelope voxel
          IMP::algebra::Vector3D voxel_center = get_location_by_voxel(l);
          envelope_voxels_centers.push_back(voxel_center);
          layer1.push_back(l);
          data_[l] = 0.0;
          grid_layer[l] = 0;
          break;
        }
      }
    }
  }
  envelope_voxels1.reserve(envelope_voxels_centers.size());
  for(unsigned int i=0; i<envelope_voxels_centers.size(); i++)
    envelope_voxels1.push_back(&envelope_voxels_centers[i]);

  // work on every layer to be computed
  IMP::algebra::Vector3D* p = NULL;
  int max_int_distance = (int)(max_distance_/header_.get_spacing() + 0.5);
  // iterate layers
  for (int layer = 0; layer < max_int_distance; layer++) {
    // std::cerr << "In Layer " << layer << " out of " << max_int_distance
    //           << " curr_layer->size() " << curr_layer->size() << std::endl;
    // update voxels with the current layer distance and
    // insert indices for next layer
    for(unsigned int i=0; i < curr_layer->size(); i++) {
      long index = (*curr_layer)[i];
      p = (*curr_envelope_voxels)[i];
      // iterate neighbor voxels
      for(unsigned int j=0; j<neighbors_.size(); j++) {
        long nindex = index + neighbors_[j];
        if(nindex >= 0 && nindex < get_number_of_voxels()) {
          float d = get_distance(get_location_by_voxel(nindex), *p);
          if(d < std::fabs(data_[nindex])) { //smaller distance, update neighbor
            if(data_[nindex] < 0) data_[nindex] = -d;
            else data_[nindex] = d;
            if(grid_layer[nindex] < layer + 1) {
              next_layer->push_back(nindex);
              next_envelope_voxels->push_back(p);
              grid_layer[nindex] = layer + 1;
            }
          }
        }
      }
    }
    // swap current and next arrays
    curr_layer->clear();
    std::vector<long> *tmp = curr_layer;
    curr_layer = next_layer; next_layer = tmp;
    curr_envelope_voxels->clear();
    std::vector<IMP::algebra::Vector3D*> *tmp_envelope_voxels =
      curr_envelope_voxels;
    curr_envelope_voxels = next_envelope_voxels;
    next_envelope_voxels = tmp_envelope_voxels;
  }
}

IMPEM_END_NAMESPACE
