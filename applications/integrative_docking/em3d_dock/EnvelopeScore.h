/**
 * \file EnvelopeScore \brief
 *
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMP_ENVELOPE_SCORE_H
#define IMP_ENVELOPE_SCORE_H

#include "MapDistanceTransform.h"

#include <vector>

class EnvelopeScore {
public:
  EnvelopeScore(const MapDistanceTransform& edt);

  bool is_penetrating(const std::vector<IMP::algebra::Vector3D>& points,
                      float penetration_thr) const;

  bool is_penetrating(const std::vector<IMP::algebra::Vector3D>& points,
                      const IMP::algebra::Transformation3D& trans,
                      float penetration_thr) const;

  int score(const std::vector<IMP::algebra::Vector3D>& points) const;
  int score(const std::vector<IMP::algebra::Vector3D>& points,
            const IMP::algebra::Transformation3D& trans) const;

private:
  unsigned int find_range(float distance) const {
    for(unsigned int i=0; i< ranges_.size(); i++)
      if(distance <= ranges_[i]) return i;
    return ranges_.size();
  }
protected:
  const MapDistanceTransform& edt_;
  std::vector<float> ranges_;
  std::vector<int> weights_;
  std::vector<long> inner_voxels_;
  std::vector<long> neighbors_;
};

#endif /* IMP_ENVELOPE_SCORE_H */
