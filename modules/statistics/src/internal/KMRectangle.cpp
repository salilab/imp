/**
 *  \file KMRectangle.cpp   \brief Orthogonal (axis aligned) rectangle
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/internal/KMRectangle.h>
#include <IMP/statistics/internal/random_generator.h>
#include <iostream>
IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE
double km_distance2(const KMPoint &p, const KMPoint &q) {
  double dist = 0;
  //IMP_INTERNAL_CHECK(); - TODO L add check for high checks level
  for (unsigned int i = 0; i < p.size(); i++) {
    dist += (p[i] - q[i]) * (p[i] - q[i]);
  }
  return dist;
}
bool km_is_equal(const KMPoint &p, const KMPoint &q) {
  for (unsigned int i = 0; i < p.size(); i++) {
    if (p[i] != q[i]) return false;
  }
  return true;
}
bool KMRectangle::is_inside(const KMPoint &p) {
  for (unsigned int i = 0; i < p.size(); i++) {
    if ((p[i] < lo_[i]) || (p[i] > hi_[i])) return false;
  }
  return true;
}

KMRectangle KMRectangle::expand(double x) {
  KMRectangle r(lo_.size());
  for (unsigned int i = 0; i < lo_.size(); i++) {
    double wid = hi_[i] - lo_[i];
    r.lo_[i] = lo_[i] - (wid / 2) * (x - 1);
    r.hi_[i] = hi_[i] + (wid / 2) * (x - 1);
  }
  return r;
}

KMPoint KMRectangle::sample() {
  KMPoint p;
  for (unsigned int i = 0; i < lo_.size(); i++)
    p.push_back(internal::random_uniform(lo_[i], hi_[i]));
  return p;
}

IMPSTATISTICS_END_INTERNAL_NAMESPACE
