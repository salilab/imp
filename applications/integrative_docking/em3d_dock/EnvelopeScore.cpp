/**
 * \file EnvelopeScore \brief
 *
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#include "EnvelopeScore.h"

//#include <IMP/atom/pdb.h>

EnvelopeScore::EnvelopeScore(const MapDistanceTransform& edt) : edt_(edt) {
  // setup distance ranges for scoring
  //[-5,-4],[-4,-3],[-3,-2],[-2,-1],[-1,1],[1,2]
  for(int i=-5; i<3; i++) if(i!=0) ranges_.push_back(i*edt_.get_spacing());
  // setup weigths
  // negative weight for negative distances outside the object
  for(int i=-6; i<-1; i++) weights_.push_back(2*(i+1));
  // weight = 1 for [-1,1] and [1-2] ranges
  weights_.push_back(1); weights_.push_back(1);weights_.push_back(1);

  for(long l=0; l<edt_.get_number_of_voxels(); l++) {
    if(edt_.get_value(l) > 2*edt_.get_spacing()) inner_voxels_.push_back(l);
  }

  // std::ofstream out_file("allvoxels.pdb");
  // int counter=1;
  // int rindex;
  // for(long l=0; l<edt_.get_number_of_voxels(); l++) {
  //   if(edt_.get_value(l) > 0) {
  //     IMP::algebra::Vector3D voxel_coordinate =
  //           edt_.get_location_by_voxel(l);
  //     rindex = (int)edt_.get_value(l);
  //     out_file << IMP::atom::get_pdb_string(voxel_coordinate,
  //     counter, IMP::atom::AT_C, IMP::atom::ALA, ' ', rindex);
  //     counter++;
  //   }
  // }
  // out_file.close();


  // init neighbour voxels offsets
  for(int x=-1; x<=1; x++) {
    for(int y=-1; y<=1; y++) {
      for(int z=-1; z<=1; z++) {
        if (x == 0 && y == 0 && z == 0) continue;
        neighbors_.push_back(
                       z*edt_.get_header()->get_nx()*edt_.get_header()->get_ny()
                       + y*edt_.get_header()->get_nx() + x);
      }
    }
  }
}

bool EnvelopeScore::is_penetrating(
                            const std::vector<IMP::algebra::Vector3D>& points,
                            float penetration_thr) const {
  for(unsigned int i=0; i<points.size(); i++) {
    float dist = edt_.get_distance_from_envelope(points[i]);
    if(dist < penetration_thr) return true;
  }
  return false;
}

bool EnvelopeScore::is_penetrating(
                             const std::vector<IMP::algebra::Vector3D>& points,
                             const IMP::algebra::Transformation3D& trans,
                             float penetration_thr) const {
  for(unsigned int i=0; i<points.size(); i++) {
    float dist = edt_.get_distance_from_envelope(trans*points[i]);
    if(dist < penetration_thr) return true;
  }
  return false;
}

int EnvelopeScore::score(const std::vector<IMP::algebra::Vector3D>& points)
  const {
  // count the number of points in each distance shell
  std::vector<int> points_in_ranges(weights_.size(), 0);
  for(unsigned int i=0; i<points.size(); i++) {
    float dist = edt_.get_distance_from_envelope(points[i]);
    unsigned int range = find_range(dist);
    points_in_ranges[range]++;
  }
  //calculate total, based on weights
  int score=0;
  for(unsigned int i=0; i<weights_.size(); i++) {
    score+= points_in_ranges[i]*weights_[i];
  }
  return score;
}

/*
  //a version that takes the map side into account
int EnvelopeScore::score(const std::vector<IMP::algebra::Vector3D>& points)
  const {
  // count the number of points in each distance shell
  std::vector<int> points_in_ranges(weights_.size(), 0);
  // mark each map voxel we hit with the point,
  // to penalize for unfilled map space
  std::vector<bool> voxel_counts(edt_.get_number_of_voxels(), false);
  for(unsigned int i=0; i<points.size(); i++) {
    float dist = edt_.get_distance_from_envelope(points[i]);
    unsigned int range = find_range(dist);
    points_in_ranges[range]++;
    if(dist >= 0.0) { // mark map voxel
      long l = edt_.get_voxel_by_location(points[i]);
      voxel_counts[l]=true;
      // iterate neighbours and mark them too
      for(unsigned int j=0; j<neighbors_.size(); j++) {
        long nindex = l + neighbors_[j];
        if(edt_.is_valid_index(nindex)) voxel_counts[nindex]=true;
      }
    }
  }

  //calculate total, based on weights
  int score=0;
  for(unsigned int i=0; i<weights_.size(); i++) {
    score+= points_in_ranges[i]*weights_[i];
  }
  // penalize for unfilled map voxels
  int penalty = 0;
  for(unsigned int i=0; i<inner_voxels_.size(); i++) {
    if(!voxel_counts[inner_voxels_[i]]) penalty++;
  }
  //  std::cerr << " penalty = " << penalty << std::endl;

  return score-(int)(penalty/4.0);
}
*/

int EnvelopeScore::score(const std::vector<IMP::algebra::Vector3D>& points,
                         const IMP::algebra::Transformation3D& trans) const {
  std::vector<IMP::algebra::Vector3D> transformed_points(points.size());
  for(unsigned int i=0; i<points.size(); i++)
    transformed_points[i] = trans * points[i];
  return score(transformed_points);
}
