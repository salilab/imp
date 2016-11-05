/**
 * \file surface_helpers.cpp
 * \brief Helper functions for surfaces.
 *
 * Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/score_functor/internal/surface_helpers.h>
#include <cmath>

IMPSCOREFUNCTOR_BEGIN_INTERNAL_NAMESPACE

FloatKey get_surface_normal_key(unsigned int i) {
  static const FloatKey surface_normal_keys[] = {FloatKey("normal_x"),
                                                 FloatKey("normal_y"),
                                                 FloatKey("normal_z")};
  return surface_normal_keys[i];
}

algebra::Vector3D get_surface_normal(Model *m, ParticleIndex pi) {
  return algebra::Vector3D(m->get_attribute(get_surface_normal_key(0), pi),
                           m->get_attribute(get_surface_normal_key(1), pi),
                           m->get_attribute(get_surface_normal_key(2), pi));
}

double get_height_above_surface(const algebra::Vector3D &center,
                                const algebra::Vector3D &normal,
                                const algebra::Vector3D &point,
                                algebra::Vector3D *dervc,
                                algebra::Vector3D *dervp) {
  if (dervc) {
    *dervc = -normal;
    *dervp = normal;
  }

  return normal * (point - center);
}

double get_depth_below_surface(const algebra::Vector3D &center,
                               const algebra::Vector3D &normal,
                               const algebra::Vector3D &point,
                               algebra::Vector3D *dervc,
                               algebra::Vector3D *dervp) {
  return get_height_above_surface(center, -normal, point, nullptr, nullptr);
}

double get_distance_from_surface(const algebra::Vector3D &center,
                                 const algebra::Vector3D &normal,
                                 const algebra::Vector3D &point,
                                 algebra::Vector3D *dervc,
                                 algebra::Vector3D *dervp) {
  double height = get_height_above_surface(center, normal, point,
                                           nullptr, nullptr);

  if (dervc) {
    double sign = (height > 0) - (height < 0);
    *dervc = -normal * sign;
    *dervp = normal * sign;
  }

  return std::abs(height);
}


IMPSCOREFUNCTOR_END_INTERNAL_NAMESPACE
