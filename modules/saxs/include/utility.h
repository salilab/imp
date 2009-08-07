/**
 *  \file saxs/utility.h
 *  \brief Functions to deal with very common saxs operations
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPSAXS_UTILITY_H
#define IMPSAXS_UTILITY_H

#include "config.h"
#include <IMP/exception.h>
#include <IMP/core/XYZ.h>

IMPSAXS_BEGIN_NAMESPACE

inline Float sinc(Float value) {
  if(std::abs(value) < 1.0e-16) return 1.0;
  return std::sin(value)/value;
}

inline void copy_coordinates(const Particles& particles,
                             std::vector<algebra::Vector3D>& coordinates) {
  // copy everything in advance for fast access
  coordinates.resize(particles.size());
  for (unsigned int i=0; i<particles.size(); i++) {
    coordinates[i] = core::XYZ(particles[i]).get_coordinates();
  }
}

inline void copy_data(const Particles& particles,
                      FormFactorTable* ff_table,
                      std::vector<algebra::Vector3D>& coordinates,
                      Floats& form_factors) {
  // copy everything in advance for fast access
  coordinates.resize(particles.size());
  form_factors.resize(particles.size());
  for (unsigned int i=0; i<particles.size(); i++) {
    coordinates[i] = core::XYZ(particles[i]).get_coordinates();
    form_factors[i] = ff_table->get_form_factor(particles[i]);
  }
}

//! compute max distance
inline Float compute_max_distance(const Particles& particles) {
  Float max_dist2 = 0;
  std::vector<algebra::Vector3D> coordinates(particles.size());
  copy_coordinates(particles, coordinates);
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates.size(); j++) {
      Float dist2 = squared_distance(coordinates[i], coordinates[j]);
      if(dist2 > max_dist2)
        max_dist2 = dist2;
    }
  }
  return sqrt(max_dist2);
}

//! compute max distance between pairs of particles one from particles1
//! and the other from particles2
inline Float compute_max_distance(const Particles& particles1,
                                  const Particles& particles2) {
  Float max_dist2 = 0;
  std::vector<algebra::Vector3D> coordinates1(particles1.size());
  std::vector<algebra::Vector3D> coordinates2(particles2.size());
  for (unsigned int i = 0; i < particles1.size(); i++) {
    coordinates1[i]
      = core::XYZ::decorate_particle(particles1[i]).get_coordinates();
  }
  for (unsigned int i = 0; i < particles2.size(); i++) {
    coordinates2[i]
      = core::XYZ::decorate_particle(particles2[i]).get_coordinates();
  }
  for (unsigned int i = 0; i < coordinates1.size(); i++) {
    for (unsigned int j = i + 1; j < coordinates2.size(); j++) {
      Float dist2 = squared_distance(coordinates1[i], coordinates2[j]);
      if(dist2 > max_dist2)
        max_dist2 = dist2;
    }
  }
  return sqrt(max_dist2);
}

//! compute radius_of_gyration
inline Float radius_of_gyration(const Particles& particles) {
  algebra::Vector3D centroid(0.0, 0.0, 0.0);
  std::vector<algebra::Vector3D> coordinates(particles.size());
  for (unsigned int i = 0; i < particles.size(); i++) {
    coordinates[i]
      = core::XYZ::decorate_particle(particles[i]).get_coordinates();
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
