/**
 *  \file DataPoints.cpp
 *  \brief Handling of data for anchor points segmentation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/internal/DataPoints.h>

IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE

void XYZDataPoints::populate_data_points(const algebra::Vector3Ds &vecs) {
  vecs_ = vecs;
  for(unsigned int i=0;i<vecs_.size();i++) {
    data_.push_back(Array1DD(3));
    for(int d=0;d<3;d++) {
      data_[i][d] = double(vecs_[i][d]);
    }
  }
}

void ParticlesDataPoints::populate_data_points(ParticlesTemp ps) {
  ps_ = get_as<Particles>(ps);
  for(unsigned int i=0;i<ps.size();i++) {
    data_.push_back(Array1DD(3));
    for(int d=0;d<3;d++) {
      data_[i][d] = double(ps[i]->get_value(FloatKey(d)));
    }
    vecs_.push_back(algebra::Vector3D(static_cast<double*>(data_.back()),
                                      static_cast<double*>(data_.back()) + 3));
  }
}

IMPSTATISTICS_END_INTERNAL_NAMESPACE
