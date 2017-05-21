/**
 * \file surface_helpers.cpp
 * \brief Helper functions for surfaces.
 *
 * Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/score_functor/internal/surface_helpers.h>
#include <cmath>

IMPSCOREFUNCTOR_BEGIN_INTERNAL_NAMESPACE

double get_height_above_surface(const algebra::Vector3D &center,
                                const algebra::Vector3D &normal,
                                const algebra::Vector3D &point,
                                algebra::Vector3D *derv) {
  if (derv) {
    *derv = normal;
  }

  return normal * (point - center);
}

double get_depth_below_surface(const algebra::Vector3D &center,
                               const algebra::Vector3D &normal,
                               const algebra::Vector3D &point,
                               algebra::Vector3D *derv) {
  return get_height_above_surface(center, -normal, point, derv);
}

double get_distance_from_surface(const algebra::Vector3D &center,
                                 const algebra::Vector3D &normal,
                                 const algebra::Vector3D &point,
                                 algebra::Vector3D *derv) {
  double height = get_height_above_surface(center, normal, point, derv);

  if (derv) {
    *derv = ((height > 0) - (height < 0)) * (*derv);
  }

  return std::abs(height);
}


IMPSCOREFUNCTOR_END_INTERNAL_NAMESPACE
