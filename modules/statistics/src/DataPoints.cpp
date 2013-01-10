/**
 *  \file DataPoints.cpp
 *  \brief Handling of data for anchor points segmentation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/DataPoints.h>
#include <IMP/core/XYZ.h>

IMPSTATISTICS_BEGIN_NAMESPACE
void XYZDataPoints::populate_data_points(const algebra::Vector3Ds &vecs) {
  vecs_ = vecs;
  int dim = atts_.size();
  for(unsigned int i=0;i<vecs_.size();i++) {
    data_.push_back(Array1DD(dim));
    for(int d=0;d<dim;d++) {
      data_[i][d] = double(vecs_[i][d]);
    }
  }
}
void ParticlesDataPoints::populate_data_points(ParticlesTemp ps) {
  ps_ = get_as<Particles>(ps);
  int dim = atts_.size();
  for(unsigned int i=0;i<ps.size();i++) {
    data_.push_back(Array1DD(dim));
    for(int d=0;d<dim;d++) {
      data_[i][d] = double(ps[i]->get_value(atts_[d]));
    }
    vecs_.push_back(core::XYZ(ps[i]).get_coordinates());
  }
}



IMPSTATISTICS_END_NAMESPACE
