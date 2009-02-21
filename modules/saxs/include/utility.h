/**
 *  \file saxs/utility.h
 *  \brief Functions to deal with very common saxs operations
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#ifndef IMPSAXS_UTILITY_H
#define IMPSAXS_UTILITY_H

#include "config.h"
#include <IMP/exception.h>

IMPSAXS_BEGIN_NAMESPACE

inline Float sinc(Float value) {
  if(fabs(value) < 1.0e-16) return 1.0;
  return sin(value)/value;
}

//! compute max distance
inline Float compute_max_distance(const std::vector<Particle*>& particles) {
  Float max_dist2 = 0;
  std::vector<algebra::Vector3D> coordinates(particles.size());
  for (unsigned int i = 0; i < particles.size(); i++) {
    coordinates[i] = core::XYZDecorator::cast(particles[i]).get_coordinates();
  }
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      Float dist2 = squared_distance(coordinates[i], coordinates[j]);
      if(dist2 > max_dist2)
        max_dist2 = dist2;
    }
  }
  return sqrt(max_dist2);
}

//! compute radius_of_gyration
inline Float radius_of_gyration(const std::vector<Particle*>& particles) {
  algebra::Vector3D centroid(0.0, 0.0, 0.0);
  std::vector<algebra::Vector3D> coordinates(particles.size());
  for (unsigned int i = 0; i < particles.size(); i++) {
    coordinates[i] = core::XYZDecorator::cast(particles[i]).get_coordinates();
    centroid += coordinates[i];
  }
  centroid /= particles.size();
  Float rg = 0;
  for (unsigned int i = 0; i < particles.size(); i++) {
    rg += squared_distance(coordinates[i], centroid);
  }
  rg /= particles.size();
  return sqrt(rg);
}

IMPSAXS_END_NAMESPACE

#endif  /* IMPSAXS_UTILITY_H */
