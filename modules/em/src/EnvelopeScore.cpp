/**
 * \file EnvelopeScore.cpp \brief
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/em/EnvelopeScore.h>

IMPEM_BEGIN_NAMESPACE

EnvelopeScore::EnvelopeScore(const MapDistanceTransform* mdt) :
  base::Object("EM_Envelope_Score"),
  mdt_(mdt) {

  // TODO: allow for more general weighting mechanism
  // setup distance ranges for scoring
  //[-5,-4],[-4,-3],[-3,-2],[-2,-1],[-1,1],[1,2]
  for(int i=-5; i<3; i++) if(i!=0) ranges_.push_back(i*mdt_->get_spacing());
  // setup weigths
  // negative weight for negative distances outside the object
  for(int i=-6; i<-1; i++) weights_.push_back(2*(i+1));
  // weight = 1 for [-1,1] and [1-2] ranges
  weights_.push_back(1); weights_.push_back(1);weights_.push_back(1);

  for(long l=0; l<mdt_->get_number_of_voxels(); l++) {
    if(mdt_->get_value(l) > 2*mdt_->get_spacing()) inner_voxels_.push_back(l);
  }

  // init neighbour voxels offsets
  for(int x=-1; x<=1; x++) {
    for(int y=-1; y<=1; y++) {
      for(int z=-1; z<=1; z++) {
        if (x == 0 && y == 0 && z == 0) continue;
        neighbors_.push_back(
                     z*mdt_->get_header()->get_nx()*mdt_->get_header()->get_ny()
                     + y*mdt_->get_header()->get_nx() + x);
      }
    }
  }
}

bool EnvelopeScore::is_penetrating(
                            const std::vector<IMP::algebra::Vector3D>& points,
                            float penetration_thr) const {
  for(unsigned int i=0; i<points.size(); i++) {
    float dist = mdt_->get_distance_from_envelope(points[i]);
    if(dist < penetration_thr) return true;
  }
  return false;
}

bool EnvelopeScore::is_penetrating(
                             const std::vector<IMP::algebra::Vector3D>& points,
                             const IMP::algebra::Transformation3D& trans,
                             float penetration_thr) const {
  for(unsigned int i=0; i<points.size(); i++) {
    float dist = mdt_->get_distance_from_envelope(trans*points[i]);
    if(dist < penetration_thr) return true;
  }
  return false;
}

double EnvelopeScore::score(const std::vector<IMP::algebra::Vector3D>& points)
  const {
  // count the number of points in each distance shell
  std::vector<int> points_in_ranges(weights_.size(), 0);
  for(unsigned int i=0; i<points.size(); i++) {
    float dist = mdt_->get_distance_from_envelope(points[i]);
    unsigned int range = find_range(dist);
    points_in_ranges[range]++;
  }
  //calculate total, based on weights
  int score=0;
  for(unsigned int i=0; i<weights_.size(); i++) {
    score+= points_in_ranges[i]*weights_[i];
  }
  return (double)score/points.size();
}

double EnvelopeScore::score(const std::vector<IMP::algebra::Vector3D>& points,
                         const IMP::algebra::Transformation3D& trans) const {
  std::vector<IMP::algebra::Vector3D> transformed_points(points.size());
  for(unsigned int i=0; i<points.size(); i++)
    transformed_points[i] = trans * points[i];
  return score(transformed_points);
}

IMPEM_END_NAMESPACE
